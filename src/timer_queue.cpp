//
// Created by wlbf on 9/4/21.
//

#include <sys/timerfd.h>
#include <glog/logging.h>
#include <cassert>
#include <vector>
#include "event_loop.h"
#include "timer_queue.h"
#include "timer.h"
#include "timer_id.h"

namespace rift::detail {
    int CreateTimerFd() {
        int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        if (timer_fd < 0) {
            LOG(FATAL) << "Failed in timerfd_create";
        }
        return timer_fd;
    }

    struct timespec HowMuchTimeFromNow(time::TimePoint when) {
        int64_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
                when - time::Now()).count();

        if (microseconds < 100) {
            microseconds = 100;
        }
        struct timespec ts{};
        ts.tv_sec = static_cast<time_t>(
                microseconds / time::micro_seconds_per_second);
        ts.tv_nsec = static_cast<long>(
                (microseconds % time::micro_seconds_per_second) * 1000);
        return ts;
    }

    void ReadTimerFd(int timer_fd, time::TimePoint now) {
        uint64_t how_many;
        ssize_t n = ::read(timer_fd, &how_many, sizeof how_many);
        VLOG(5) << "TimerQueue::handleRead() " << how_many << " at "
                << std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
        if (n != sizeof how_many) {
            LOG(ERROR) << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
        }
    }

    void ResetTimerFd(int timer_fd, time::TimePoint expiration) {
        // wake up loop by timerfd_settime()
        struct itimerspec newValue{};
        struct itimerspec oldValue{};
        bzero(&newValue, sizeof newValue);
        bzero(&oldValue, sizeof oldValue);
        newValue.it_value = HowMuchTimeFromNow(expiration);
        int ret = ::timerfd_settime(timer_fd, 0, &newValue, &oldValue);
        if (ret) {
            LOG(ERROR) << "timerfd_settime()";
        }
    }

}

using namespace rift;
using namespace rift::detail;

TimerQueue::TimerQueue(EventLoop *loop)
        : loop_(loop),
          timer_fd_(CreateTimerFd()),
          timer_fd_channel_(loop, timer_fd_),
          timers_() {
    timer_fd_channel_.SetReadCallback([this](time::TimePoint) { HandleRead(); });
    // we are always reading the timer_fd, we disarm it with timerfd_settime.
    timer_fd_channel_.EnableReading();
}

TimerQueue::~TimerQueue() {
    ::close(timer_fd_);
    // do not remove channel, since we're in  EventLoop::destructor();
}

TimerID TimerQueue::addTimer(const TimerCallback &cb,
                             time::TimePoint when,
                             double interval) {
    auto *timer = new Timer(cb, when, interval);
    loop_->RunInLoop([this, when, timer] { AddTimerInLoop(when, timer); });
    return TimerID(timer, timer->sequence_);
}

void TimerQueue::AddTimerInLoop(time::TimePoint when, Timer *timer) {
    loop_->AssetInLoopThread();
    auto timer_id = TimerID(timer, timer->sequence_);
    bool earliest_changed = Insert(Entry{{when, timer_id}, timer});

    if (earliest_changed) {
        ResetTimerFd(timer_fd_, timer->Expiration().value());
    }
}

void TimerQueue::HandleRead() {
    loop_->AssetInLoopThread();
    time::TimePoint now = time::Now();
    ReadTimerFd(timer_fd_, now);

    std::vector<Entry> expired = GetExpired(now);

    calling_expired_timers_ = true;
    canceling_timers_.clear();
    // safe to call outside critical section
    for (auto &it: expired) {
        it.second->Run();
    }
    calling_expired_timers_ = false;

    Reset(std::move(expired), now);
}

std::vector<TimerQueue::Entry> TimerQueue::GetExpired(time::TimePoint now) {
    auto timer_id = TimerID(reinterpret_cast<Timer *>(UINTPTR_MAX), INT64_MAX);
    auto it = timers_.lower_bound(Key(now, timer_id));
    assert(it == timers_.end() || now < it->first.first);
    std::vector<Entry> expired(std::make_move_iterator(timers_.begin()), std::make_move_iterator(it));
    timers_.erase(timers_.begin(), it);

    for (auto &entry: expired) {
        size_t n = active_timers_.erase(entry.first.second);
        assert(n == 1);
    }

    assert(timers_.size() == active_timers_.size());
    return expired;
}

void TimerQueue::Reset(std::vector<Entry> &&expired, time::TimePoint now) {
    for (auto &entry: expired) {
        if (entry.second->Repeat() && canceling_timers_.find(entry.first.second) == canceling_timers_.end()) {
            entry.second->Restart(now);
            Insert(std::move(entry));
        }
    }

    if (!timers_.empty()) {
        auto next_expire = timers_.begin()->second->Expiration();
        if (next_expire.has_value()) {
            ResetTimerFd(timer_fd_, next_expire.value());
        }
    }
}

bool TimerQueue::Insert(TimerQueue::Entry &&entry) {
    bool earliest_changed = false;
    assert(entry.second->Expiration().has_value());
    time::TimePoint when = entry.second->Expiration().value();
    auto it = timers_.begin();
    if (it == timers_.end() || when < it->first.first) {
        earliest_changed = true;
    }

    {
        auto[_, ok] = active_timers_.insert(entry.first.second);
        assert(ok);
    }
    {
        auto[_, ok] = timers_.insert(std::move(entry));
        assert(ok);
    }

    assert(timers_.size() == active_timers_.size());
    return earliest_changed;
}

void TimerQueue::Cancel(TimerID timer_id) {
    loop_->RunInLoop([this, timer_id] { CancelInLoop(timer_id); });
}

void TimerQueue::CancelInLoop(TimerID timer_id) {
    loop_->AssetInLoopThread();
    assert(timers_.size() == active_timers_.size());
    auto it = active_timers_.find(timer_id);
    if (it != active_timers_.end()) {
        size_t n = timers_.erase(Key(timer_id.timer_->Expiration().value(), timer_id));
        assert(n == 1);
        active_timers_.erase(it);
    } else if (calling_expired_timers_) {
        canceling_timers_.insert(timer_id);
    }
    assert(timers_.size() == active_timers_.size());
}

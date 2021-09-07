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
#include "time_point.h"

namespace rift::detail {
    int CreateTimerFd() {
        int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        if (timer_fd < 0) {
            LOG(ERROR) << "Failed in timerfd_create";
        }
        return timer_fd;
    }

    struct timespec HowMuchTimeFromNow(TimePoint when) {
        int64_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
                when - std::chrono::system_clock::now()).count();

        if (microseconds < 100) {
            microseconds = 100;
        }
        struct timespec ts{};
        ts.tv_sec = static_cast<time_t>(
                microseconds / MicroSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(
                (microseconds % MicroSecondsPerSecond) * 1000);
        return ts;
    }

    void ReadTimerFd(int timer_fd, TimePoint now) {
        uint64_t how_many;
        ssize_t n = ::read(timer_fd, &how_many, sizeof how_many);
        LOG(INFO) << "TimerQueue::handleRead() " << how_many << " at "
                  << std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
        if (n != sizeof how_many) {
            LOG(ERROR) << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
        }
    }

    void ResetTimerFd(int timer_fd, TimePoint expiration) {
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
    timer_fd_channel_.SetReadCallback([this] { HandleRead(); });
    // we are always reading the timer_fd, we disarm it with timerfd_settime.
    timer_fd_channel_.EnableReading();
}

TimerQueue::~TimerQueue() {
    ::close(timer_fd_);
    // do not remove channel, since we're in  EventLoop::destructor();
}

TimerId TimerQueue::addTimer(const TimerCallback &cb,
                             TimePoint when,
                             double interval) {
    auto *timer = new Timer(cb, when, interval);
    loop_->RunInLoop([this, when, timer] { AddTimerInLoop(when, timer); });
    return TimerId(timer);
}

void TimerQueue::AddTimerInLoop(TimePoint when, Timer *timer) {
    loop_->AssetInLoopThread();
    bool earliest_changed = Insert(Entry{when, timer});

    if (earliest_changed) {
        ResetTimerFd(timer_fd_, timer->Expiration().value());
    }
}

void TimerQueue::HandleRead() {
    loop_->AssetInLoopThread();
    TimePoint now = std::chrono::system_clock::now();
    ReadTimerFd(timer_fd_, now);

    std::vector<Entry> expired = GetExpired(now);

    // safe to call outside critical section
    for (auto &it : expired) {
        it.second->Run();
    }

    Reset(std::move(expired), now);
}

std::vector<TimerQueue::Entry> TimerQueue::GetExpired(TimePoint now) {
    auto it = timers_.lower_bound(now);
    assert(it == timers_.end() || now < it->first);
    std::vector<Entry> expired(std::make_move_iterator(timers_.begin()), std::make_move_iterator(it));
    timers_.erase(timers_.begin(), it);
    return expired;
}

void TimerQueue::Reset(std::vector<Entry> &&expired, TimePoint now) {
    for (auto &entry : expired) {
        if (entry.second->Repeat()) {
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
    TimePoint when = entry.second->Expiration().value();
    auto it = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        earliest_changed = true;
    }

    timers_.insert(std::move(entry));

    return earliest_changed;
}

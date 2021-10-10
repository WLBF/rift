//
// Created by wlbf on 8/29/21.
//

#include "event_loop.h"
#include "poller.h"
#include "channel.h"
#include "timer_queue.h"

#include <cassert>
#include <glog/logging.h>
#include <signal.h>
#include <sys/eventfd.h>


namespace rift {

    thread_local EventLoop *t_loop_in_this_thread = nullptr;
    const int k_poll_time_ms = 10000;

    static int CreateEventFd() {
        int evt_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (evt_fd < 0) {
            LOG(FATAL) << "Failed in eventfd";
        }
        return evt_fd;
    }

    class IgnoreSigPipe {
    public:
        IgnoreSigPipe() {
            ::signal(SIGPIPE, SIG_IGN);
        }
    };

    [[maybe_unused]] IgnoreSigPipe init_obj; // Disable SIGPIPE

    EventLoop::EventLoop()
            : looping_(false), quit_(false),
              thread_id_(std::this_thread::get_id()),
              calling_pending_fucntors_(false),
              poller_(new Poller(this)),
              timer_queue_(new TimerQueue(this)),
              wakeup_fd_(CreateEventFd()),
              wakeup_channel_(new Channel(this, wakeup_fd_)) {
        VLOG(5) << "EventLoop created " << this << " in thread " << thread_id_;
        if (t_loop_in_this_thread) {
            LOG(FATAL) << "Another EventLoop " << t_loop_in_this_thread
                       << " exists in this thread " << thread_id_;
        } else {
            t_loop_in_this_thread = this;
        }
        wakeup_channel_->SetReadCallback([this](time::TimePoint) { HandleRead(); });
        // we are always reading the wakeup_fd
        wakeup_channel_->EnableReading();
    }

    EventLoop::~EventLoop() {
        assert(!looping_);
        t_loop_in_this_thread = nullptr;
    }

    void EventLoop::AbortNotInLoopTread() {
        LOG(FATAL) << "EventLoop::AbortNotInLoopThread - EventLoop " << this
                   << " was created in threadId_ = " << thread_id_
                   << ", current thread id = " << std::this_thread::get_id();
    }

    EventLoop *EventLoop::GetEventLoopOfCurrentThread() {
        return t_loop_in_this_thread;
    }

    void EventLoop::Loop() {
        assert(!looping_);
        AssetInLoopThread();
        looping_ = true;
        quit_ = false;

        while (!quit_) {
            active_channels_.clear();
            poll_return_time_ = poller_->Poll(k_poll_time_ms, &active_channels_);
            for (auto &ch: active_channels_) {
                ch->HandleEvent(poll_return_time_);
            }
            DoPendingFunctors();
        }

        VLOG(5) << "EventLoop " << this << " stop looping";
        looping_ = false;
    }

    void EventLoop::Quit() {
        quit_ = true;
        if (!IsInLoopThread()) {
            Wakeup();
        }
    }

    void EventLoop::UpdateChannel(Channel *channel) {
        assert(channel->OwnerLoop() == this);
        AssetInLoopThread();
        poller_->UpdateChannel(channel);
    }

    void EventLoop::RemoveChannel(Channel *channel) {
        assert(channel->OwnerLoop() == this);
        AssetInLoopThread();
        poller_->RemoveChannel(channel);
    }

    TimerId EventLoop::RunAt(const time::TimePoint &time, const TimerCallback &cb) {
        return timer_queue_->addTimer(cb, time, 0);
    }

    TimerId EventLoop::RunAfter(double delay, const TimerCallback &cb) {
        auto time = time::Now() + time::Duration(delay);
        return RunAt(time, cb);
    }

    TimerId EventLoop::RunEvery(double interval, const TimerCallback &cb) {
        auto time = time::Now() + time::Duration(interval);
        return timer_queue_->addTimer(cb, time, interval);
    }

    void EventLoop::RunInLoop(const EventLoop::Functor &cb) {
        if (IsInLoopThread()) {
            cb();
        } else {
            QueueInLoop(cb);
        }
    }

    void EventLoop::QueueInLoop(const EventLoop::Functor &cb) {
        {
            std::scoped_lock lock(mutex_);
            pending_functors_.push_back(cb);
        }

        if (!IsInLoopThread() || calling_pending_fucntors_) {
            Wakeup();
        }
    }

    void EventLoop::Wakeup() const {
        uint64_t one = 1;
        ssize_t n = ::write(wakeup_fd_, &one, sizeof one);
        if (n != sizeof one) {
            LOG(ERROR) << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
        }
    }

    void EventLoop::HandleRead() const {
        uint64_t one = 1;
        ssize_t n = ::read(wakeup_fd_, &one, sizeof one);
        if (n != sizeof one) {
            LOG(ERROR) << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
        }
    }

    void EventLoop::DoPendingFunctors() {
        std::vector<Functor> functors;
        calling_pending_fucntors_ = true;
        {
            std::scoped_lock lock(mutex_);
            functors.swap(pending_functors_);
        }

        for (auto &f: functors) {
            f();
        }

        calling_pending_fucntors_ = false;
    }

}
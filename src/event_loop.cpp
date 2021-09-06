//
// Created by wlbf on 8/29/21.
//

#include "event_loop.h"
#include "poller.h"
#include "channel.h"
#include "timer_queue.h"

#include <cassert>
#include <glog/logging.h>


namespace rift {

    thread_local EventLoop *t_loop_in_this_thread = nullptr;
    const int k_poll_time_ms = 10000;

    EventLoop::EventLoop()
            : looping_(false), quit_(false),
              thread_id_(std::this_thread::get_id()),
              poller_(new Poller(this)),
              timer_queue_(new TimerQueue(this)) {
        LOG(INFO) << "EventLoop created " << this << " in thread " << thread_id_;
        if (t_loop_in_this_thread) {
            LOG(INFO) << "Another EventLoop " << t_loop_in_this_thread
                      << " exists in this thread " << thread_id_;
            abort();
        } else {
            t_loop_in_this_thread = this;
        }
    }

    EventLoop::~EventLoop() {
        assert(!looping_);
        t_loop_in_this_thread = nullptr;
    }

    void EventLoop::AbortNotInLoopTread() {
        LOG(FATAL) << "EventLoop::AbortNotInLoopThread - EventLoop " << this
                   << " was created in threadId_ = " << thread_id_
                   << ", current thread id = " << std::this_thread::get_id();
        abort();
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
            for (auto &ch : active_channels_) {
                ch->HandleEvent();
            }
        }

        LOG(INFO) << "EventLoop " << this << " stop looping";
        looping_ = false;
    }

    void EventLoop::Quit() {
        quit_ = true;
    }

    void EventLoop::UpdateChannel(Channel *channel) {
        assert(channel->OwnerLoop() == this);
        AssetInLoopThread();
        poller_->UpdateChannel(channel);
    }

    TimerId EventLoop::RunAt(const TimePoint &time, const TimerCallback &cb) {
        return timer_queue_->addTimer(cb, time, 0);
    }

    TimerId EventLoop::RunAfter(double delay, const TimerCallback &cb) {
        auto time = std::chrono::system_clock::now() + Duration(delay);
        return RunAt(time, cb);
    }

    TimerId EventLoop::RunEvery(double interval, const TimerCallback &cb) {
        auto time = std::chrono::system_clock::now() + Duration(interval);
        return timer_queue_->addTimer(cb, time, interval);
    }
}
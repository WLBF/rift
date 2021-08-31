//
// Created by wlbf on 8/29/21.
//

#include <cassert>
#include <poll.h>
#include <glog/logging.h>
#include "event_loop.h"

namespace rift {

    thread_local EventLoop *t_loop_in_this_thread = nullptr;

    EventLoop::EventLoop() : looping_(false), thread_id_(std::this_thread::get_id()) {
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
        LOG(FATAL) << "EventLoop::abortNotInLoopThread - EventLoop " << this
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

        ::poll(nullptr, 0, 5 * 1000);

        LOG(INFO) << "EventLoop " << this << " stop looping";
        looping_ = false;
    }
}
//
// Created by wlbf on 9/8/21.
//

#include "event_loop_thread.h"
#include "event_loop.h"
#include <cassert>

namespace rift {

    EventLoopThread::EventLoopThread()
            : loop_(nullptr),
              exiting_(false),
              thread_(),
              mutex_(),
              cond_() {}

    EventLoopThread::~EventLoopThread() {
        exiting_ = true;
        loop_->Quit();
        thread_.join();
    }

    EventLoop *EventLoopThread::StartLoop() {
        assert(!thread_.joinable());

        thread_ = std::thread([this] { ThreadFunc(); });
        {
            std::unique_lock<std::mutex> lk(mutex_);
            cond_.wait(lk, [&] { return loop_ != nullptr; });
        }

        return loop_;
    }

    void EventLoopThread::ThreadFunc() {
        EventLoop loop, *p_loop = &loop;
        {
            std::unique_lock<std::mutex> lk(mutex_);
            loop_ = p_loop;
            cond_.notify_all();
        }

        loop.Loop();
    }
}

//
// Created by wlbf on 10/11/21.
//

#include "event_loop.h"
#include "event_loop_thread.h"
#include "event_loop_thread_pool.h"
#include <cassert>

namespace rift {

    EventLoopThreadPool::EventLoopThreadPool(EventLoop *base_loop)
            : base_loop_(base_loop),
              started_(false),
              num_threads_(0),
              next_(0) {}


    // Don't delete loop, it's a stack variable
    EventLoopThreadPool::~EventLoopThreadPool() = default;

    void EventLoopThreadPool::Start() {
        assert(!started_);
        base_loop_->AssetInLoopThread();
        started_ = true;

        for (int i = 0; i < num_threads_; i++) {
            auto *t = new EventLoopThread();
            threads_.emplace_back(t);
            loops_.push_back(t->StartLoop());
        }
    }

    EventLoop *EventLoopThreadPool::GetNextLoop() {
        base_loop_->AssetInLoopThread();
        EventLoop *loop = base_loop_;

        if (!threads_.empty()) {
            loop = loops_[next_++];
            if (next_ >= threads_.size()) {
                next_ = 0;
            }
        }
        return loop;
    }
}

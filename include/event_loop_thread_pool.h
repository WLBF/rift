//
// Created by wlbf on 10/11/21.
//

#ifndef RIFT_EVENT_LOOP_THREAD_POOL_H
#define RIFT_EVENT_LOOP_THREAD_POOL_H

#include <vector>
#include <memory>

namespace rift {
    class EventLoop;

    class EventLoopThread;

    class EventLoopThreadPool {
    public:
        explicit EventLoopThreadPool(EventLoop *base_loop);

        ~EventLoopThreadPool();

        void SetThreadNum(int num_threads) { num_threads_ = num_threads; }

        void Start();

        EventLoop *GetNextLoop();

    private:
        EventLoop *base_loop_;
        bool started_;
        int num_threads_;
        int next_; // always in loop thread

        std::vector<std::unique_ptr<EventLoopThread>> threads_;
        std::vector<EventLoop *> loops_;
    };
}


#endif //RIFT_EVENT_LOOP_THREAD_POOL_H

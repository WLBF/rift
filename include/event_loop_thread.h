//
// Created by wlbf on 9/8/21.
//

#ifndef RIFT_EVENTLOOPTHREAD_H
#define RIFT_EVENTLOOPTHREAD_H

#include <thread>
#include <mutex>
#include <condition_variable>

namespace rift {
    class EventLoop;

    class EventLoopThread {
    public:
        EventLoopThread();

        ~EventLoopThread();

        EventLoop *StartLoop();

    private:
        void ThreadFunc();

        EventLoop *loop_;
        bool exiting_;
        std::thread thread_;
        std::mutex mutex_;
        std::condition_variable cond_;
    };
}


#endif //RIFT_EVENTLOOPTHREAD_H

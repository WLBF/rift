//
// Created by wlbf on 8/29/21.
//

#ifndef RIFT_EVENTLOOP_H
#define RIFT_EVENTLOOP_H

#include <thread>

namespace rift {

    class EventLoop {
    public:
        EventLoop();

        EventLoop(const EventLoop &) = delete;

        EventLoop(EventLoop &&) = delete;

        EventLoop &operator=(const EventLoop &) = delete;

        ~EventLoop();

        void AssetInLoopThread() {
            if (!IsInLoopThread()) {
                AbortNotInLoopTread();
            }
        }

        bool IsInLoopThread() const {
            return thread_id_ == std::this_thread::get_id();
        }

        EventLoop *GetEventLoopOfCurrentThread();

        void Loop();

    private:
        void AbortNotInLoopTread();

        bool looping_;
        const std::thread::id thread_id_;
    };
}

#endif //RIFT_EVENTLOOP_H

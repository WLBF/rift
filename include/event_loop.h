//
// Created by wlbf on 8/29/21.
//

#ifndef RIFT_EVENTLOOP_H
#define RIFT_EVENTLOOP_H

#include <thread>
#include <vector>

namespace rift {

    class Channel;

    class Poller;

    class EventLoop {
    public:
        EventLoop();

        EventLoop(const EventLoop &) = delete;

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

        ///
        /// Loops forever.
        ///
        /// Must be called in the same thread as creation of the object.
        //
        void Loop();

        void Quit();

        void UpdateChannel(Channel *channel);

    private:
        void AbortNotInLoopTread();

        using ChannelList = std::vector<Channel *>;

        bool looping_; /* atomic */
        bool quit_; /* atomic */
        const std::thread::id thread_id_;
        std::unique_ptr<Poller> poller_;
        ChannelList active_channels_{};
    };
}

#endif //RIFT_EVENTLOOP_H

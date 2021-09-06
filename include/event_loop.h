//
// Created by wlbf on 8/29/21.
//

#ifndef RIFT_EVENTLOOP_H
#define RIFT_EVENTLOOP_H

#include "callbacks.h"
#include "time_point.h"
#include "timer_id.h"
#include <thread>
#include <vector>

namespace rift {

    class Channel;

    class Poller;

    class TimerQueue;

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

        [[nodiscard]] bool IsInLoopThread() const {
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

        ///
        /// Runs callback at 'time'.
        ///
        TimerId RunAt(const TimePoint &time, const TimerCallback &cb);

        ///
        /// Runs callback after @c delay seconds.
        ///
        TimerId RunAfter(double delay, const TimerCallback &cb);

        ///
        /// Runs callback every @c interval seconds.
        ///
        TimerId RunEvery(double interval, const TimerCallback &cb);

        void UpdateChannel(Channel *channel);

    private:
        void AbortNotInLoopTread();

        using ChannelList = std::vector<Channel *>;

        bool looping_; /* atomic */
        bool quit_; /* atomic */
        const std::thread::id thread_id_;
        TimePoint poll_return_time_;
        std::unique_ptr<Poller> poller_;
        std::unique_ptr<TimerQueue> timer_queue_;
        ChannelList active_channels_{};
    };
}

#endif //RIFT_EVENTLOOP_H

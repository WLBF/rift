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
#include <mutex>

namespace rift {

    class Channel;

    class Poller;

    class TimerQueue;

    class EventLoop {
    public:
        using Functor = std::function<void()>;

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

        static EventLoop *GetEventLoopOfCurrentThread();

        ///
        /// Loops forever.
        ///
        /// Must be called in the same thread as creation of the object.
        //
        void Loop();

        void Quit();

        ///
        /// Time when poll returns, usually means data arrival.
        ///
        [[nodiscard]] time::TimePoint PollReturnTime() const { return poll_return_time_; }

        /// Runs callback immediately in the loop thread.
        /// It wakes up the loop, and run the cb.
        /// If in the same loop thread, cb is run within the function.
        /// Safe to call from other threads.
        void RunInLoop(const Functor &cb);

        /// Queues callback in the loop thread.
        /// Runs after finish pooling.
        /// Safe to call from other threads.
        void QueueInLoop(const Functor &cb);

        ///
        /// Runs callback at 'time'.
        ///
        TimerId RunAt(const time::TimePoint &time, const TimerCallback &cb);

        ///
        /// Runs callback after @c delay seconds.
        ///
        TimerId RunAfter(double delay, const TimerCallback &cb);

        ///
        /// Runs callback every @c interval seconds.
        ///
        TimerId RunEvery(double interval, const TimerCallback &cb);

        // internal use only
        void Wakeup() const;

        void UpdateChannel(Channel *channel);

        void RemoveChannel(Channel *channel);

    private:
        using ChannelList = std::vector<Channel *>;

        void AbortNotInLoopTread();

        void HandleRead() const;

        void DoPendingFunctors();


        bool looping_; /* atomic */
        bool quit_; /* atomic */
        bool calling_pending_fucntors_; /* atomic */
        const std::thread::id thread_id_;
        time::TimePoint poll_return_time_;
        std::unique_ptr<Poller> poller_;
        std::unique_ptr<TimerQueue> timer_queue_;
        int wakeup_fd_;
        // unlike in TimerQueue, which is an internal class,
        // we don't expose Channel to client
        std::unique_ptr<Channel> wakeup_channel_;
        ChannelList active_channels_{};
        std::mutex mutex_;
        std::vector<Functor> pending_functors_; // @GuardedBy mutex_
    };
}

#endif //RIFT_EVENTLOOP_H

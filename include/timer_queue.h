//
// Created by wlbf on 9/4/21.
//

#ifndef RIFT_TIMER_QUEUE_H
#define RIFT_TIMER_QUEUE_H

#include <memory>
#include <map>
#include "callbacks.h"
#include "time_point.h"
#include "channel.h"

namespace rift {

    class EventLoop;

    class Timer;

    class TimerId;

    ///
    /// A best efforts timer queue.
    /// No guarantee that the callback will be on time.
    //
    class TimerQueue {
    public:
        explicit TimerQueue(EventLoop *loop);

        ~TimerQueue();

        ///
        /// Schedules the callback to be run at given time,
        /// repeats if @c interval > 0.0.
        ///
        /// Must be thread safe. Usually be called from other threads.
        TimerId addTimer(const TimerCallback &cb, TimePoint when, double interval);


    private:
        using Entry = std::pair<TimePoint, std::unique_ptr<Timer>>;
        using TimerList = std::multimap<TimePoint, std::unique_ptr<Timer>>;

        // called when timer_fd alarms
        void HandleRead();

        // move out all expired timers
        std::vector<Entry> GetExpired(TimePoint now);

        void Reset(std::vector<Entry> &&expired, TimePoint now);

        bool Insert(Entry &&entry);

        EventLoop *loop_;
        const int timer_fd_;
        Channel timer_fd_channel_;
        // Timer list sorted by expiration;
        TimerList timers_;

        void AddTimerInLoop(TimePoint when, Timer *timer);
    };

}


#endif //RIFT_TIMER_QUEUE_H

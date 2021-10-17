//
// Created by wlbf on 9/4/21.
//

#ifndef RIFT_TIMER_QUEUE_H
#define RIFT_TIMER_QUEUE_H

#include <memory>
#include <map>
#include <set>

#include "callbacks.h"
#include "time_point.h"
#include "channel.h"

namespace rift {

    class EventLoop;

    class Timer;

    class TimerID;

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
        TimerID addTimer(const TimerCallback &cb, time::TimePoint when, double interval);

        void Cancel(TimerID timer_id);


    private:
        using Key = std::pair<time::TimePoint, TimerID>;
        using Entry = std::pair<Key, std::unique_ptr<Timer>>;
        using TimerList = std::map<Key, std::unique_ptr<Timer>>;
        using ActiveTimerSet = std::set<TimerID>;

        void AddTimerInLoop(time::TimePoint when, Timer *timer);

        void CancelInLoop(TimerID timer_id);

        // called when timer_fd alarms
        void HandleRead();

        // move out all expired timers
        std::vector<Entry> GetExpired(time::TimePoint now);

        void Reset(std::vector<Entry> &&expired, time::TimePoint now);

        bool Insert(Entry &&entry);

        EventLoop *loop_;
        const int timer_fd_;
        Channel timer_fd_channel_;
        // Timer list sorted by expiration;
        TimerList timers_;

        // for Cancel()
        bool calling_expired_timers_{}; /* atomic */
        ActiveTimerSet active_timers_;
        ActiveTimerSet canceling_timers_;
    };

}


#endif //RIFT_TIMER_QUEUE_H

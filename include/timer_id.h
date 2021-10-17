//
// Created by wlbf on 9/4/21.
//

#ifndef RIFT_TIMER_ID_H
#define RIFT_TIMER_ID_H

namespace rift {
    class Timer;

    ///
    /// An opaque identifier, for canceling Timer.
    ///
    class TimerID {
    public:
        explicit TimerID(Timer *timer, int64_t seq) : timer_(timer), sequence_(seq) {}

        // default copy-constructor, destructor and assignment are okay

        bool operator==(const TimerID &timer_id) {
            return timer_id.timer_ == timer_ && timer_id.sequence_ == sequence_;
        }

        bool operator<(const TimerID &timer_id) const {
            return timer_id.timer_ < timer_ && timer_id.sequence_ < sequence_;
        }

        friend class TimerQueue;

    private:
        Timer *timer_;
        int64_t sequence_;
    };

}

#endif //RIFT_TIMER_ID_H

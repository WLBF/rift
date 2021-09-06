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
    class TimerId {
    public:
        explicit TimerId(Timer *timer) : value_(timer) {}

        // default copy-constructor, destructor and assignment are okay

    private:
        Timer *value_;
    };

}

#endif //RIFT_TIMER_ID_H

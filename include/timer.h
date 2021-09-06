//
// Created by wlbf on 9/4/21.
//

#ifndef RIFT_TIMER_H
#define RIFT_TIMER_H

#include "callbacks.h"
#include "time_point.h"
#include <chrono>
#include <utility>
#include <optional>

namespace rift {

    ///
    /// Internal class for timer event.
    ///
    class Timer {
    public:
        Timer(TimerCallback cb, TimePoint when, double interval)
                : callback_(std::move(cb)),
                  expiration_(when),
                  interval_(interval), repeat_(interval > 0.0) {}

        Timer(const Timer &) = delete;

        Timer &operator=(const Timer &) = delete;

        void Run() const {
            callback_();
        }

        [[nodiscard]] std::optional<TimePoint> Expiration() const { return expiration_; }

        [[nodiscard]] bool Repeat() const { return repeat_; }

        void Restart(TimePoint now);

    private:
        const TimerCallback callback_;
        std::optional<TimePoint> expiration_;
        const double interval_;
        const bool repeat_;
    };
}


#endif //RIFT_TIMER_H

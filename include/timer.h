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
#include <atomic>

namespace rift {

    ///
    /// Internal class for timer event.
    ///
    class Timer {
    public:
        Timer(TimerCallback cb, time::TimePoint when, double interval)
                : callback_(std::move(cb)),
                  expiration_(when),
                  interval_(interval),
                  repeat_(interval > 0.0),
                  sequence_(seq_num_created_.fetch_add(1)) {}

        Timer(const Timer &) = delete;

        Timer &operator=(const Timer &) = delete;

        void Run() const {
            callback_();
        }

        [[nodiscard]] std::optional<time::TimePoint> Expiration() const { return expiration_; }

        [[nodiscard]] bool Repeat() const { return repeat_; }

        void Restart(time::TimePoint now);

        friend class TimerQueue;

    private:
        const TimerCallback callback_;
        std::optional<time::TimePoint> expiration_;
        const double interval_;
        const bool repeat_;
        const int64_t sequence_;

        static std::atomic_int64_t seq_num_created_;
    };
}


#endif //RIFT_TIMER_H

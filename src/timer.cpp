//
// Created by wlbf on 9/4/21.
//

#include "timer.h"

namespace rift {
    std::atomic_int64_t Timer::seq_num_created_;

    void Timer::Restart(time::TimePoint now) {
        if (repeat_) {
            expiration_ = now + time::Duration(interval_);
        } else {
            expiration_ = std::nullopt;
        }
    }
}

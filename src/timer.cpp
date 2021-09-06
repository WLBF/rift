//
// Created by wlbf on 9/4/21.
//

#include "timer.h"

namespace rift {
    void Timer::Restart(TimePoint now) {
        if (repeat_) {
            expiration_ = now + Duration (interval_);
        } else {
            expiration_ = std::nullopt;
        }
    }
}

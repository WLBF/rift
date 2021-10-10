//
// Created by wlbf on 9/5/21.
//

#ifndef RIFT_TIME_POINT_H
#define RIFT_TIME_POINT_H

#include <chrono>
#include <string>

namespace rift::time {
    using Duration = std::chrono::duration<double>;
    using TimePoint = std::chrono::time_point<std::chrono::system_clock, Duration>;

    static const int micro_seconds_per_second = 1000 * 1000;

    TimePoint Now();

    std::string ToFormattedString(TimePoint when);
}

#endif //RIFT_TIME_POINT_H

//
// Created by wlbf on 9/5/21.
//

#ifndef RIFT_TIME_POINT_H
#define RIFT_TIME_POINT_H

#include <chrono>

namespace rift {
    using Duration = std::chrono::duration<double>;
    using TimePoint = std::chrono::time_point<std::chrono::system_clock, Duration>;

    static const int MicroSecondsPerSecond = 1000 * 1000;
}

#endif //RIFT_TIME_POINT_H

//
// Created by wlbf on 10/10/21.
//

#include "time_point.h"

namespace rift::time {
    TimePoint Now() {
        return std::chrono::system_clock::now();
    }

    std::string ToFormattedString(TimePoint when) {
        std::time_t when_c = std::chrono::system_clock::to_time_t(
                std::chrono::time_point_cast<std::chrono::nanoseconds>(when));
        std::tm when_tm = *std::localtime(&when_c);

        int64_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
                when.time_since_epoch()).count();
        int ms = static_cast<int>(microseconds % micro_seconds_per_second);

        char buf[64] = {0};
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d\n",
                 when_tm.tm_year + 1900, when_tm.tm_mon + 1, when_tm.tm_mday,
                 when_tm.tm_hour, when_tm.tm_min, when_tm.tm_sec, ms);

        return buf;
    }
}

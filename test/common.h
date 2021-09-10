//
// Created by wlbf on 9/10/21.
//

#ifndef RIFT_COMMON_H
#define RIFT_COMMON_H

#include <thread>
#include <cstdio>
#include <unistd.h>
#include <sstream>

uint64_t GetTid() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return std::stoull(ss.str());
}

#endif //RIFT_COMMON_H

//
// Created by wlbf on 9/6/21.
//

#include <chrono>
#include <thread>
#include "event_loop.h"
#include <gtest/gtest.h>
#include <glog/logging.h>

int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(EventLoop, Basic) {
    rift::EventLoop loop, *p_loop = &loop;

    int cnt = 0;
    auto print = [&](const char *msg) {
        printf("msg %s\n", msg);
        if (++cnt == 20) {
            p_loop->Quit();
        }
    };

    print("main");
    loop.RunAfter(1, [&]() { print("once1"); });
    loop.RunAfter(1.5, [&]() { print("once1.5"); });
    loop.RunAfter(2.5, [&]() { print("once2.5"); });
    loop.RunAfter(3.5, [&]() { print("once3.5"); });
    loop.RunEvery(2, [&]() { print("every2"); });
    loop.RunEvery(3, [&]() { print("every3"); });

    loop.Loop();
    print("main loop exits");
    std::this_thread::sleep_for(std::chrono::seconds(1));
}


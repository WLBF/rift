//
// Created by wlbf on 8/29/21.
//

#include <gtest/gtest.h>
#include <glog/logging.h>
#include "event_loop.h"

int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(EventLoop, Basic) {
    rift::EventLoop loop;

    std::thread thr([]() {
        rift::EventLoop loop;
        loop.Loop();
    });

    loop.Loop();
    thr.join();
}

TEST(EventLoop, CreateTwice) {
    rift::EventLoop loop1;
    ASSERT_DEATH({ rift::EventLoop loop2; }, "");
}

TEST(EventLoopDeathTest, CallLoopInOtherThread) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    rift::EventLoop loop;

    std::thread thr([&]() {
        ASSERT_DEATH({ loop.Loop(); }, "");
    });

    thr.join();
}

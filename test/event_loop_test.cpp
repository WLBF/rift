//
// Created by wlbf on 8/29/21.
//

#include <chrono>
#include <thread>
#include "event_loop.h"
#include <gtest/gtest.h>
#include <glog/logging.h>
#include <sys/timerfd.h>
#include "channel.h"

int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(EventLoop, Basic) {
    rift::EventLoop loop;

    std::thread thr([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        loop.Quit();
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

TEST(EventLoop, PollBasic) {
    rift::EventLoop loop, *p_loop = &loop;

    std::function<void()> timeout = [=](){
        p_loop->Quit();
    };

    int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    rift::Channel channel(p_loop, timer_fd);

    channel.SetReadCallback(timeout);
    channel.EnableReading();

    struct itimerspec how_long{};
    bzero(&how_long, sizeof how_long);
    how_long.it_value.tv_sec = 5;

    ::timerfd_settime(timer_fd, 0, &how_long, nullptr);
    loop.Loop();
    ::close(timer_fd);
}

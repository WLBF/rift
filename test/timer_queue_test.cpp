//
// Created by wlbf on 9/6/21.
//

#include <chrono>
#include <thread>
#include "event_loop.h"
#include "event_loop_thread.h"
#include <gtest/gtest.h>
#include <glog/logging.h>
#include <cstdio>
#include <unistd.h>

int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

uint64_t GetTid() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return std::stoull(ss.str());
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

TEST(EventLoop, RunAfter) {
    rift::EventLoop loop, *p_loop = &loop;
    int flag = 0;

    auto run4 = [&] {
        printf("run4(): pid = %d, flag = %d\n", getpid(), flag);
        p_loop->Quit();
    };

    auto run3 = [&] {
        printf("run3(): pid = %d, flag = %d\n", getpid(), flag);
        p_loop->RunAfter(3, run4);
        flag = 3;
    };

    auto run2 = [&] {
        printf("run2(): pid = %d, flag = %d\n", getpid(), flag);
        p_loop->RunAfter(3, run3);
    };

    auto run1 = [&] {
        flag = 1;
        printf("run1(): pid = %d, flag = %d\n", getpid(), flag);
        p_loop->RunAfter(3, run2);
        flag = 2;
    };

    printf("main(): pid = %d, flag = %d\n", getpid(), flag);

    loop.RunAfter(2, run1);
    loop.Loop();
    printf("main(): pid = %d, flag = %d\n", getpid(), flag);
}

TEST(EventLoop, EventLoopThread) {

    auto run_in_thread = [] {
        printf("runInThread(): pid = %d, tid = %lu\n", getpid(), GetTid());
    };

    printf("main(): pid = %d, tid = %lu\n", getpid(), GetTid());

    rift::EventLoopThread loop_thread;
    rift::EventLoop *loop = loop_thread.StartLoop();
    loop->RunInLoop(run_in_thread);
    sleep(1);
    loop->RunAfter(2, run_in_thread);
    sleep(3);
    loop->Quit();

    printf("exit main().\n");;
}

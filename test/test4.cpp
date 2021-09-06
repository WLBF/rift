//
// Created by wlbf on 9/6/21.
//

#include "event_loop.h"

#include <thread>
#include <glog/logging.h>

int cnt = 0;

void print(const char *msg) {
}

int main() {
    ::google::InitGoogleLogging("");

    rift::EventLoop loop, *p_loop = &loop;

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


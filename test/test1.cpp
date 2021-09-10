#include "event_loop.h"
#include "common.h"
#include <cstdio>
#include <unistd.h>
#include <glog/logging.h>


int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);
    auto thread_func = []() {
        printf("thread_func(): pid = %d, tid = %lu\n",
               getpid(), GetTid());

        rift::EventLoop loop;
        loop.Loop();
    };


    printf("main(): pid = %d, tid = %lu\n", getpid(), GetTid());

    rift::EventLoop loop;

    std::thread thr(thread_func);

    loop.Loop();
}

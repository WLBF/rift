#include "event_loop.h"
#include "channel.h"
#include <glog/logging.h>


int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);

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

#include "event_loop.h"
#include "event_loop_thread.h"
#include "channel.h"
#include "common.h"
#include <glog/logging.h>


int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);

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

#include "event_loop.h"
#include <glog/logging.h>

int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);
    rift::EventLoop loop;

    std::thread thr([&]() {
        loop.Loop();
    });

    thr.join();
}

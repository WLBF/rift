#include "event_loop.h"
#include "channel.h"
#include <unistd.h>
#include <glog/logging.h>
#include <sys/timerfd.h>


int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);
    printf("%f started!\n", rift::time::Now().time_since_epoch().count());
    rift::EventLoop loop, *p_loop = &loop;

    std::function<void(rift::time::TimePoint)> timeout = [=](rift::time::TimePoint receive_time) {
        printf("%f Timeout!\n", rift::time::Now().time_since_epoch().count());
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

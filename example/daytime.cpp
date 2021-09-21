//
// Created by wlbf on 9/20/21.
//

#include "acceptor.h"
#include "event_loop.h"
#include "inet_address.h"
#include "socket.h"
#include "cstdio"
#include <glog/logging.h>
#include <chrono>


int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);

    auto new_connection = [](rift::SocketPtr &&sock, const rift::InetAddress peer_addr) {
        printf("newConnection(): accepted a new connection from %s\n", peer_addr.ToHostPort().c_str());

        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm = *std::localtime(&now_c);

        char buf[64] = {0};
        int len = snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d\n",
                           now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday,
                           now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec);

        ::write(sock->Fd(), buf, len);
    };

    printf("main(): pid = %d\n", getpid());

    rift::InetAddress listen_addr(9981);
    rift::EventLoop loop;

    rift::Acceptor acceptor(&loop, listen_addr);
    acceptor.SetNewConnectionCallback(new_connection);
    acceptor.Listen();

    loop.Loop();
}

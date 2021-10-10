//
// Created by wlbf on 9/20/21.
//

#include "acceptor.h"
#include "event_loop.h"
#include "inet_address.h"
#include "socket.h"
#include "cstdio"
#include <glog/logging.h>


int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);

    auto new_connection = [](rift::SocketPtr &&sock, const rift::InetAddress peer_addr) {
        printf("newConnection(): accepted a new connection from %s\n", peer_addr.ToHostPort().c_str());

        auto now = rift::time::Now();
        auto time = rift::time::ToFormattedString(now);

        ::write(sock->Fd(), time.c_str(), time.length());
    };

    printf("main(): pid = %d\n", getpid());

    rift::InetAddress listen_addr(9981);
    rift::EventLoop loop;

    rift::Acceptor acceptor(&loop, listen_addr);
    acceptor.SetNewConnectionCallback(new_connection);
    acceptor.Listen();

    loop.Loop();
}

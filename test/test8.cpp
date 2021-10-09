//
// Created by wlbf on 9/21/21.
//

#include "tcp_server.h"
#include "event_loop.h"
#include "inet_address.h"
#include "tcp_connection.h"
#include <cstdio>
#include <glog/logging.h>

void OnConnection(const rift::TcpConnectionPtr &conn) {
    if (conn->Connected()) {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->Name().c_str(),
               conn->PeerAddress().ToHostPort().c_str());
    } else {
        printf("onConnection(): connection [%s] is down\n",
               conn->Name().c_str());
    }
}

void OnMessage(const rift::TcpConnectionPtr &conn,
               rift::Buffer* buf,
               rift::TimePoint receive_time) {
    printf("onMessage(): received %zd bytes from connection [%s]\n",
           buf->ReadableBytes(), conn->Name().c_str());

    printf("OnMessage(): [%s]\n", buf->RetrieveAsString().c_str());
}

int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);
    printf("main(): pid = %d\n", getpid());

    rift::InetAddress listen_addr(9981);
    rift::EventLoop loop;

    rift::TcpServer server(&loop, listen_addr);
    server.SetConnectionCallback(OnConnection);
    server.SetMessageCallback(OnMessage);
    server.Start();

    loop.Loop();
}

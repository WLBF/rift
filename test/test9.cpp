//
// Created by wlbf on 10/10/21.
//

#include "tcp_server.h"
#include "event_loop.h"
#include "inet_address.h"
#include "tcp_connection.h"
#include <cstdio>
#include <unistd.h>
#include <glog/logging.h>

void OnConnection(const rift::TcpConnectionPtr &conn) {
    if (conn->Connected()) {
        printf("OnConnection(): new connection [%s] from %s\n", conn->Name().c_str(),
               conn->PeerAddress().ToHostPort().c_str());
    } else {
        printf("OnConnection(): connection [%s] is down\n",
               conn->Name().c_str());
    }
}

void onMessage(const rift::TcpConnectionPtr &conn,
               rift::Buffer *buf,
               rift::time::TimePoint receive_time) {
    printf("OnMessage(): received %zd bytes from connection [%s] at %s\n",
           buf->ReadableBytes(),
           conn->Name().c_str(),
           rift::time::ToFormattedString(receive_time).c_str());

    conn->Send(buf->RetrieveAsString());
}

int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);
    printf("main(): pid = %d\n", getpid());

    rift::InetAddress listenAddr(9981);
    rift::EventLoop loop;

    rift::TcpServer server(&loop, listenAddr);
    server.SetConnectionCallback(OnConnection);
    server.SetMessageCallback(onMessage);
    server.Start();

    loop.Loop();
}

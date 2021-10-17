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

std::string message1;
std::string message2;

void OnConnection(const rift::TcpConnectionPtr &conn) {
    if (conn->Connected()) {
        printf("OnConnection(): new connection [%s] from %s\n",
               conn->Name().c_str(),
               conn->PeerAddress().ToHostPort().c_str());
        conn->Send(message1);
        conn->Send(message2);
        conn->Shutdown();
    } else {
        printf("OnConnection(): connection [%s] is down\n", conn->Name().c_str());
    }
}

void OnMessage(const rift::TcpConnectionPtr &conn,
               rift::Buffer *buf,
               rift::time::TimePoint receive_time) {
    printf("OnMessage(): received %zd bytes from connection [%s] at %s\n",
           buf->ReadableBytes(),
           conn->Name().c_str(),
           rift::time::ToFormattedString(receive_time).c_str());

    buf->RetrieveAll();
}


int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);
    printf("main(): pid = %d\n", getpid());

    int len1 = 100;
    int len2 = 200;

    if (argc > 2) {
        len1 = atoi(argv[1]);
        len2 = atoi(argv[2]);
    }

    message1.resize(len1);
    message2.resize(len2);
    std::fill(message1.begin(), message1.end(), 'A');
    std::fill(message2.begin(), message2.end(), 'B');

    rift::InetAddress listenAddr(9981);
    rift::EventLoop loop;

    rift::TcpServer server(&loop, listenAddr);
    server.SetConnectionCallback(OnConnection);
    server.SetMessageCallback(OnMessage);
    server.Start();

    loop.Loop();
}

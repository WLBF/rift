//
// Created by wlbf on 10/10/21.
//

#include "tcp_server.h"
#include "tcp_connection.h"
#include "event_loop.h"
#include "inet_address.h"
#include <cstdio>
#include <unistd.h>

std::string message;

void OnConnection(const rift::TcpConnectionPtr &conn) {
    if (conn->Connected()) {
        printf("OnConnection(): new connection [%s] from %s\n",
               conn->Name().c_str(),
               conn->PeerAddress().ToHostPort().c_str());
        conn->Send(message);
    } else {
        printf("onConnection(): connection [%s] is down\n", conn->Name().c_str());
    }
}

void OnWriteComplete(const rift::TcpConnectionPtr &conn) {
    conn->Send(message);
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

int main() {
    printf("main(): pid = %d\n", getpid());

    std::string line;
    for (int i = 33; i < 127; ++i) {
        line.push_back(char(i));
    }
    line += line;

    for (size_t i = 0; i < 127 - 33; ++i) {
        message += line.substr(i, 72) + '\n';
    }

    rift::InetAddress listenAddr(9981);
    rift::EventLoop loop;

    rift::TcpServer server(&loop, listenAddr);
    server.SetConnectionCallback(OnConnection);
    server.SetMessageCallback(OnMessage);
    server.SetWriteCompleteCallback(OnWriteComplete);
    server.Start();

    loop.Loop();
}


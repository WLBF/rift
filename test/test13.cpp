//
// Created by wlbf on 10/18/21.
//

#include "event_loop.h"
#include "inet_address.h"
#include "tcp_client.h"

#include <cstdio>
#include <glog/logging.h>

static std::string message = "Hello\n";

void OnConnection(const rift::TcpConnectionPtr &conn) {
    if (conn->Connected()) {
        printf("OnConnection(): new connection [%s] from %s\n",
               conn->Name().c_str(),
               conn->PeerAddress().ToHostPort().c_str());
        conn->Send(message);
    } else {
        printf("OnConnection(): connection [%s] is down\n",
               conn->Name().c_str());
    }
}

void OnMessage(const rift::TcpConnectionPtr &conn,
               rift::Buffer *buf,
               rift::time::TimePoint receive_time) {
    printf("OnMessage(): received %zd bytes from connection [%s] at %s\n",
           buf->ReadableBytes(),
           conn->Name().c_str(),
           rift::time::ToFormattedString(receive_time).c_str());

    printf("OnMessage(): [%s]\n", buf->RetrieveAsString().c_str());
}

int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);
    rift::EventLoop loop;
    rift::InetAddress serverAddr("localhost", 9981);
    rift::TcpClient client(&loop, serverAddr);

    client.SetConnectionCallback(OnConnection);
    client.SetMessageCallback(OnMessage);
    client.EnableRetry();
    client.Connect();
    loop.Loop();
}


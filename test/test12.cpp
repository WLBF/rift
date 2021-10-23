//
// Created by wlbf on 10/17/21.
//

#include "connector.h"
#include "event_loop.h"
#include <cstdio>
#include <glog/logging.h>

int main(int argc, char **argv) {
    ::google::InitGoogleLogging(argv[0]);
    rift::EventLoop loop;
    auto connect_callback = [&](rift::SocketPtr &&sock) {
        printf("connected.\n");
        loop.Quit();
    };

    rift::InetAddress addr("127.0.0.1", 9981);
    rift::ConnectorPtr connector(new rift::Connector(&loop, addr));
    connector->SetNewConnectionCallback(connect_callback);
    connector->Start();

    loop.Loop();
}


//
// Created by wlbf on 9/10/21.
//

#include "acceptor.h"
#include "event_loop.h"
#include "inet_address.h"
#include "socket_ops.h"
#include <glog/logging.h>

namespace rift {

    Acceptor::Acceptor(EventLoop *loop, const InetAddress &listen_addr)
            : loop_(loop),
              accept_socket_(sockets::CreateNonblockingOrDie()),
              accept_channel_(loop, accept_socket_.Fd()),
              listening_(false) {
        accept_socket_.SetReuseAddr(true);
        accept_socket_.BindAddress(listen_addr);
        accept_channel_.SetReadCallback([this] { HandleRead(); });
    }

    void Acceptor::Listen() {
        loop_->AssetInLoopThread();
        listening_ = true;
        accept_socket_.Listen();
        accept_channel_.EnableReading();
    }

    void Acceptor::HandleRead() {
        loop_->AssetInLoopThread();
        InetAddress peer_addr(0);
        //FIXME loop until no more
        int conn_fd = accept_socket_.Accept(&peer_addr);
        SocketPtr conn_sock = std::make_unique<Socket>(conn_fd);
        if (new_connection_callback_) {
            new_connection_callback_(std::move(conn_sock), peer_addr);
        } else {
            sockets::Close(conn_fd);
        }
    }
}

//
// Created by wlbf on 9/21/21.
//

#include "tcp_connection.h"
#include "socket.h"
#include "channel.h"
#include "event_loop.h"

#include <cassert>
#include <utility>
#include <glog/logging.h>

namespace rift {
    TcpConnection::TcpConnection(EventLoop *loop, std::string name, SocketPtr &&sock, const InetAddress &local_addr,
                                 const InetAddress &peer_addr)
            : loop_(loop),
              name_(std::move(name)),
              state_(k_connecting),
              socket_(std::move(sock)),
              channel_(new Channel(loop, socket_->Fd())),
              local_addr_(local_addr),
              peer_addr_(peer_addr) {

        LOG(INFO) << "TcpConnection::ctor[" << name_ << "] at " << this
                  << " fd=" << socket_->Fd();
        channel_->SetReadCallback([this] { HandleRead(); });
    }

    TcpConnection::~TcpConnection() {
        LOG(INFO) << "TcpConnection::dtor[" << name_ << "] at " << this
                  << " fd=" << channel_->Fd();
    }

    void TcpConnection::ConnectEstablished() {
        loop_->AssetInLoopThread();
        assert(state_ == StateE::k_connecting);
        state_ = StateE::k_connected;
        channel_->EnableReading();
        connection_callback_(shared_from_this());
    }

    void TcpConnection::HandleRead() {
        char buf[65536];
        ssize_t n = ::read(channel_->Fd(), buf, sizeof buf);
        message_callback_(shared_from_this(), buf, n);
        // FIXME: close connection if n == 0
    }
}
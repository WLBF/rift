//
// Created by wlbf on 9/21/21.
//

#include "tcp_connection.h"
#include "socket.h"
#include "socket_ops.h"
#include "channel.h"
#include "event_loop.h"

#include <cassert>
#include <cerrno>
#include <utility>
#include <glog/logging.h>
#include <cstdio>

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

        VLOG(4) << "TcpConnection::ctor[" << name_ << "] at " << this
                  << " fd=" << socket_->Fd();
        channel_->SetReadCallback([this] { HandleRead(); });
    }

    TcpConnection::~TcpConnection() {
        VLOG(4)  << "TcpConnection::dtor[" << name_ << "] at " << this
                  << " fd=" << channel_->Fd();
    }

    void TcpConnection::ConnectEstablished() {
        loop_->AssetInLoopThread();
        assert(state_ == StateE::k_connecting);
        state_ = StateE::k_connected;
        channel_->EnableReading();
        connection_callback_(shared_from_this());
    }

    void TcpConnection::ConnectDestroyed() {
        loop_->AssetInLoopThread();
        assert(state_ == k_connected);
        SetState(k_disconnected);
        channel_->DisableAll();
        connection_callback_(shared_from_this());
        loop_->RemoveChannel(channel_.get());
    }

    void TcpConnection::HandleRead() {
        char buf[65536];
        ssize_t n = ::read(channel_->Fd(), buf, sizeof buf);
        if (n > 0) {
            message_callback_(shared_from_this(), buf, n);
        } else if (n == 0) {
            HandleClose();
        } else {
            HandleError();
        }
    }

    void TcpConnection::HandleWrite() {

    }

    void TcpConnection::HandleClose() {
        loop_->AssetInLoopThread();
        VLOG(5) << "TcpConnection::HandleClose state = " << state_;
        assert(state_ == k_connected);
        // we don't close fd, leave it  to destructor, so we can find leaks easily.
        channel_->DisableAll();
        // must be the last line;
        close_callback_(shared_from_this());
    }

    void TcpConnection::HandleError() {
        int err = sockets::GetSocketError(channel_->Fd());
        LOG(ERROR) << "TcpConnection::HandleError [" << name_
                   << "] - SO_ADDR = " << err << " " << strerror(err);
    }
}

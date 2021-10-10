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
        channel_->SetReadCallback([this](time::TimePoint receive_time) { HandleRead(receive_time); });
    }

    TcpConnection::~TcpConnection() {
        VLOG(4) << "TcpConnection::dtor[" << name_ << "] at " << this
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
        assert(state_ == k_connected || state_ == k_disconnecting);
        SetState(k_disconnected);
        channel_->DisableAll();
        connection_callback_(shared_from_this());
        loop_->RemoveChannel(channel_.get());
    }

    void TcpConnection::HandleRead(time::TimePoint receive_time) {
        int saved_errno = 0;
        ssize_t n = input_buffer_.ReadFd(channel_->Fd(), &saved_errno);
        if (n > 0) {
            message_callback_(shared_from_this(), &input_buffer_, receive_time);
        } else if (n == 0) {
            HandleClose();
        } else {
            errno = saved_errno;
            LOG(ERROR) << "TcpConnection::HandleRead";
            HandleError();
        }
    }

    void TcpConnection::HandleWrite() {
        loop_->AssetInLoopThread();
        if (channel_->IsWriting()) {
            ssize_t n = ::write(channel_->Fd(), output_buffer_.Peek(), output_buffer_.ReadableBytes());
            if (n > 0) {
                output_buffer_.Retrieve(n);
                if (output_buffer_.ReadableBytes() == 0) {
                    channel_->DisableWriting();
                    if (state_ == k_disconnecting) {
                        ShutDownInLoop();
                    }
                } else {
                    VLOG(5) << "I am going to write more data";
                }
            } else {
                LOG(ERROR) << "TcpConnection::HandleWrite";
            }
        } else {
            VLOG(5) << "Connection is down, no more writing";
        }
    }

    void TcpConnection::HandleClose() {
        loop_->AssetInLoopThread();
        VLOG(5) << "TcpConnection::HandleClose state = " << state_;
        assert(state_ == k_connected || state_ == k_disconnecting);
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

    void TcpConnection::Send(const std::string &message) {
        // FIXME: maybe move message here
        if (state_ == k_connected) {
            if (loop_->IsInLoopThread()) {
                SendInLoop(message);
            } else {
                // FIXME: shared_from_this?
                loop_->RunInLoop([this, message] { SendInLoop(message); });
            }
        }
    }

    void TcpConnection::Shutdown() {
        // FIXME: use compare and swap
        if (state_ == k_connected) {
            SetState(k_disconnecting);
            loop_->RunInLoop([ptr = shared_from_this()] { ptr->ShutDownInLoop(); });
        }
    }

    void TcpConnection::SendInLoop(const std::string &message) {
        loop_->AssetInLoopThread();
        ssize_t nwrote = 0;
        // if nothing in output queue, try to write directly
        if (!channel_->IsWriting() && output_buffer_.ReadableBytes() == 0) {
            nwrote = ::write(channel_->Fd(), message.data(), message.size());
            if (nwrote >= 0) {
                if (nwrote < message.size()) {
                    VLOG(5) << "I am going to write more data";
                }
            } else {
                nwrote = 0;
                if (errno != EWOULDBLOCK) {
                    LOG(ERROR) << "TcpConnection::SendInLoop";
                }
            }
        }

        assert(nwrote >= 0);
        if (nwrote < message.size()) {
            output_buffer_.Append(message.data(), message.size());
            if (!channel_->IsWriting()) {
                channel_->EnableReading();
            }
        }
    }

    void TcpConnection::ShutDownInLoop() {
        loop_->AssetInLoopThread();
        if (!channel_->IsWriting()) {
            // we are not writing
            socket_->ShutdownWrite();
        }
    }

    void TcpConnection::SetTcpNoDelay(bool on) {
        socket_->SetTcpNoDelay(on);
    }
}

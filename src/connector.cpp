//
// Created by wlbf on 10/14/21.
//

#include "channel.h"
#include "connector.h"
#include "event_loop.h"
#include "inet_address.h"
#include "socket_ops.h"
#include <glog/logging.h>
#include <cassert>
#include <memory>

namespace rift {
    const int Connector::k_max_retry_delay_ms;

    Connector::Connector(EventLoop *loop, InetAddress &server_addr)
            : loop_(loop),
              server_addr_(server_addr),
              connect_(false),
              state_(k_disconnected),
              retry_delay_ms_(k_init_retry_delay_ms),
              timer_id_(nullptr, 0) {
        VLOG(4) << "constructor[" << this << "]";

    }

    Connector::~Connector() {
        VLOG(4) << "constructor[" << this << "]";
        loop_->Cancel(timer_id_);
        assert(!channel_);
    }

    void Connector::Start() {
        connect_ = true;
        loop_->RunInLoop([this] { Connect(); });
    }

    void Connector::Stop() {
        connect_ = false;
        loop_->Cancel(timer_id_);
    }

    void Connector::Restart() {
        loop_->AssetInLoopThread();
        SetState(k_disconnected);
        retry_delay_ms_ = k_init_retry_delay_ms;
        connect_ = true;
        StartInLoop();
    }

    void Connector::StartInLoop() {
        loop_->AssetInLoopThread();
        assert(state_ == k_disconnected);
        if (connect_) {
            Connect();
        } else {
            VLOG(4) << "do not connect";
        }
    }

    void Connector::Connect() {
        int sock_fd = sockets::CreateNonblockingOrDie();
        int ret = sockets::Connect(sock_fd, server_addr_.GetSockAddrInet());
        int savedErrno = (ret == 0) ? 0 : errno;
        switch (savedErrno) {
            case 0:
            case EINPROGRESS:
            case EINTR:
            case EISCONN:
                Connecting(sock_fd);
                break;

            case EAGAIN:
            case EADDRINUSE:
            case EADDRNOTAVAIL:
            case ECONNREFUSED:
            case ENETUNREACH:
                Retry(sock_fd);
                break;

            case EACCES:
            case EPERM:
            case EAFNOSUPPORT:
            case EALREADY:
            case EBADF:
            case EFAULT:
            case ENOTSOCK:
                LOG(ERROR) << "connect error in Connector::StartInLoop " << savedErrno;
                sockets::Close(sock_fd);
                break;

            default:
                LOG(ERROR) << "Unexpected error in Connector::StartInLoop " << savedErrno;
                sockets::Close(sock_fd);
                // connectErrorCallback_();
                break;
        }
    }

    void Connector::Connecting(int sock_fd) {
        SetState(k_connecting);
        assert(!channel_);
        channel_ = std::make_unique<Channel>(loop_, sock_fd);
        channel_->SetWriteCallback([this] { HandleWrite(); });
        channel_->SetErrorCallback([this] { HandleError(); });

        // channel_->tie(shared_from_this()); is not working,
        // as channel_ is not managed by shared_ptr
        channel_->EnableWriting();
    }

    void Connector::HandleWrite() {

        VLOG(5) << "Connector::HandleWrite " << state_;

        if (state_ == k_connecting) {
            int sock_fd = RemoveAndResetChannel();
            int err = sockets::GetSocketError(sock_fd);
            if (err) {
                LOG(WARNING) << "Connector::HandleWrite - SO_ERROR = "
                             << err << " " << strerror(err);
                Retry(sock_fd);
            } else if (sockets::IsSelfConnect(sock_fd)) {
                LOG(WARNING) << "Connector::HandleWrite - Self connect";
                Retry(sock_fd);
            } else {
                SetState(k_connected);
                if (connect_) {
                    new_connection_callback_(sock_fd);
                } else {
                    sockets::Close(sock_fd);
                }
            }
        } else {
            // Connector is in retry progress.
            assert(state_ == k_disconnected);
        }
    }

    void Connector::HandleError() {
        LOG(ERROR) << "Connector::HandleError";
        assert(state_ == k_connecting);

        int sock_fd = RemoveAndResetChannel();
        int err = sockets::GetSocketError(sock_fd);
        VLOG(5) << "SO_ERROR = " << err << " " << strerror(err);
        Retry(sock_fd);
    }

    void Connector::Retry(int sock_fd) {
        sockets::Close(sock_fd);
        SetState(k_disconnected);
        if (connect_) {
            VLOG(0) << "Connector::retry - Retry connecting to "
                    << server_addr_.ToHostPort() << " in "
                    << retry_delay_ms_ << " milliseconds. ";
            timer_id_ = loop_->RunAfter(retry_delay_ms_ / 1000.0,  // FIXME: unsafe
                                        [this] { StartInLoop(); });
            retry_delay_ms_ = std::min(retry_delay_ms_ * 2, k_max_retry_delay_ms);
        } else {
            VLOG(4) << "do not connect";
        }
    }

    int Connector::RemoveAndResetChannel() {
        channel_->DisableAll();
        loop_->RemoveChannel(channel_.get());
        int sock_fd = channel_->Fd();
        // Can't reset channel_ here, because we are inside Channel::handleEvent
        loop_->QueueInLoop([this] { ResetChannel(); });
        return sock_fd;
    }

    void Connector::ResetChannel() {
        channel_.reset();
    }
}

//
// Created by wlbf on 9/21/21.
//

#ifndef RIFT_TCP_CONNECTION_H
#define RIFT_TCP_CONNECTION_H

#include "inet_address.h"
#include "callbacks.h"
#include "socket.h"
#include "buffer.h"
#include <memory>

namespace rift {
    class Channel;

    class EventLoop;

    class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
    public:
        /// Constructs a TcpConnection with a connected sock
        ///
        /// User should not create this object.
        TcpConnection(EventLoop *loop, std::string name, SocketPtr &&sock, const InetAddress &local_addr,
                      const InetAddress &peer_addr);

        TcpConnection(const TcpConnection &) = delete;

        TcpConnection &operator=(const TcpConnection &) = delete;

        ~TcpConnection();

        EventLoop *GetLoop() const { return loop_; }

        const std::string &Name() const { return name_; }

        const InetAddress &LocalAddress() { return local_addr_; }

        const InetAddress &PeerAddress() { return peer_addr_; }

        bool Connected() const { return state_ == k_connected; }

        void SetConnectionCallback(const ConnectionCallback &cb) { connection_callback_ = cb; }

        void SetMessageCallback(const MessageCallback &cb) { message_callback_ = cb; }

        void SetCloseCallback(const CloseCallback &cb) { close_callback_ = cb; }

        /// Internal use only.

        // called when TcpServer accepts a new connection
        void ConnectEstablished();   // should be called only once

        // called when TcpServer has removed me from its map
        void ConnectDestroyed(); // should be called only once

    private:
        enum StateE {
            k_connecting, k_connected, k_disconnected,
        };

        void SetState(StateE s) { state_ = s; }

        void HandleRead(time::TimePoint receive_time);

        void HandleWrite();

        void HandleClose();

        void HandleError();

        EventLoop *loop_;
        std::string name_;
        StateE state_;  // FIXME: use atomic variable
        // we don't expose those classes to client.
        SocketPtr socket_;
        std::unique_ptr<Channel> channel_;
        InetAddress local_addr_;
        InetAddress peer_addr_;
        ConnectionCallback connection_callback_;
        MessageCallback message_callback_;
        CloseCallback close_callback_;
        Buffer input_buffer_;
    };

    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
}

#endif //RIFT_TCP_CONNECTION_H

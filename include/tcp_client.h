//
// Created by wlbf on 10/18/21.
//

#ifndef RIFT_TCP_CLIENT_H
#define RIFT_TCP_CLIENT_H

#include "tcp_connection.h"
#include <memory>
#include <mutex>

namespace rift {

    class Connector;

    using ConnectorPtr = std::shared_ptr<Connector>;

    class TcpClient {
    public:
        TcpClient(EventLoop *loop, InetAddress &server_addr);

        TcpClient(const TcpClient &) = delete;

        TcpClient &operator=(const TcpClient &) = delete;

        ~TcpClient(); // force out-line destructor, for scoped_ptr members.

        void Connect();

        void Disconnect();

        void Stop();

        [[nodiscard]] TcpConnectionPtr connection() {
            std::scoped_lock lock(mutex_);
            return connection_;
        }

        [[nodiscard]] bool Retry() const { return retry_; };

        void EnableRetry() { retry_ = true; }

        /// Set connection callback.
        /// Not thread safe.
        void SetConnectionCallback(const ConnectionCallback &cb) { connection_callback_ = cb; }

        /// Set message callback.
        /// Not thread safe.
        void SetMessageCallback(const MessageCallback &cb) { message_callback_ = cb; }

        /// Set write complete callback.
        /// Not thread safe.
        void setWriteCompleteCallback(const WriteCompleteCallback &cb) { write_complete_callback_ = cb; }

    private:
        /// Not thread safe, but in loop
        void NewConnection(SocketPtr &&sock);

        /// Not thread safe, but in loop
        void RemoveConnection(const TcpConnectionPtr &conn);

        EventLoop *loop_;
        ConnectorPtr connector_;
        ConnectionCallback connection_callback_;
        MessageCallback message_callback_;
        WriteCompleteCallback write_complete_callback_;
        bool retry_;   // atmoic
        bool connect_; // atomic
        // always in loop thread
        int next_conn_id_;
        std::mutex mutex_;
        TcpConnectionPtr connection_; // @BuardedBy mutex_
    };

}

#endif //RIFT_TCP_CLIENT_H

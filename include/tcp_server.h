//
// Created by wlbf on 9/21/21.
//

#ifndef RIFT_TCP_SERVER_H
#define RIFT_TCP_SERVER_H

#include "callbacks.h"
#include "socket.h"
#include "inet_address.h"


#include <map>
#include <memory>

namespace rift {
    class EventLoop;

    class Acceptor;

    class EventLoopThreadPool;

    class TcpServer {
    public:
        TcpServer(EventLoop *loop, const InetAddress &listen_addr);

        TcpServer(const TcpServer &) = delete;

        TcpServer &operator=(const TcpServer &) = delete;

        ~TcpServer();

        /// Starts the server if it's not listening.
        ///
        /// It's harmless to call it multiple times.
        /// Thread safe.
        void Start();

        /// Set connection callback.
        /// Not thread safe.
        void SetConnectionCallback(const ConnectionCallback &cb) { connection_callback_ = cb; }

        /// Set message callback.
        /// Not thread safe.
        void SetMessageCallback(const MessageCallback &cb) { message_callback_ = cb; }

        /// Set write complete callback.
        /// Not thread safe.
        void SetWriteCompleteCallback(const WriteCompleteCallback &cb) { write_complete_callback_ = cb; }

        /// Set the number of threads for handling input.
        ///
        /// @param num_threads
        /// - 0 means all I/O in loop's thread, no thread will created.
        ///   this is the default value.
        /// - 1 means all I/O in anther thread.
        /// - N means a thread pool with N threads, new connections are assigned on a round-robin basis.
        void SetThreadNum(int num_threads);

    private:
        using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

        /// Not thread safe, but in loop
        void NewConnection(SocketPtr &&sock, const InetAddress &peer_addr);

        void RemoveConnection(const TcpConnectionPtr &conn);

        void RemoveConnectionInLoop(const TcpConnectionPtr &conn);

        EventLoop *loop_; // the Acceptor loop
        const std::string name_;
        std::unique_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
        std::unique_ptr<EventLoopThreadPool> thread_pool_;
        MessageCallback message_callback_;
        ConnectionCallback connection_callback_;
        WriteCompleteCallback write_complete_callback_;
        bool started_;
        int next_conn_id_; // always in loop thread
        ConnectionMap connections_;
    };
}

#endif //RIFT_TCP_SERVER_H

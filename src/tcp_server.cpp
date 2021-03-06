//
// Created by wlbf on 9/21/21.
//

#include "acceptor.h"
#include "event_loop.h"
#include "event_loop_thread_pool.h"
#include "tcp_server.h"
#include "socket_ops.h"
#include "tcp_connection.h"

#include <cassert>
#include <glog/logging.h>

namespace rift {

    TcpServer::TcpServer(EventLoop *loop, const InetAddress &listen_addr)
            : loop_(loop),
              name_(listen_addr.ToHostPort()),
              acceptor_(new Acceptor(loop, listen_addr)),
              thread_pool_(new EventLoopThreadPool(loop)),
              started_(false),
              next_conn_id_(1) {
        acceptor_->SetNewConnectionCallback(
                [this](SocketPtr &&sock, const InetAddress &addr) { NewConnection(std::move(sock), addr); });
    }

    TcpServer::~TcpServer() = default;

    void TcpServer::Start() {
        if (!started_) {
            started_ = true;
        }

        if (!acceptor_->Listening()) {
            loop_->RunInLoop([this] { acceptor_->Listen(); });
        }
    }

    void TcpServer::NewConnection(SocketPtr &&sock, const InetAddress &peer_addr) {
        loop_->AssetInLoopThread();
        char buf[32];
        snprintf(buf, sizeof buf, "#%d", next_conn_id_);
        next_conn_id_++;
        std::string conn_name = name_ + buf;

        VLOG(0) << "TcpServer::NewConnection [" << name_
                << "] - new connection [" << conn_name
                << "] from " << peer_addr.ToHostPort();

        // FIXME poll with zero timeout to double confirm the new connection.
        InetAddress local_addr(sockets::GetLocalAddr(sock->Fd()));

        EventLoop *io_loop = thread_pool_->GetNextLoop();
        TcpConnectionPtr conn = std::make_shared<TcpConnection>(io_loop, conn_name, std::move(sock), local_addr,
                                                                peer_addr);
        connections_[conn_name] = conn;
        conn->SetConnectionCallback(connection_callback_);
        conn->SetMessageCallback(message_callback_);
        conn->SetWriteCompleteCallback(write_complete_callback_);
        conn->SetCloseCallback([this](const TcpConnectionPtr &conn) { RemoveConnection(conn); });
        conn->ConnectEstablished();
    }

    void TcpServer::RemoveConnection(const TcpConnectionPtr &conn) {
        // FIXME: unsafe
        loop_->RunInLoop([this, conn] { RemoveConnectionInLoop(conn); });
    }

    void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &conn) {
        loop_->AssetInLoopThread();
        VLOG(0) << "TcpServer::RemoveConnection [" << name_
                << "] - connection " << conn->Name();
        size_t n = connections_.erase(conn->Name());
        assert(n == 1);
        loop_->QueueInLoop([conn] { conn->ConnectDestroyed(); });
    }

    void TcpServer::SetThreadNum(int num_threads) {
        thread_pool_->SetThreadNum(num_threads);
    }
}

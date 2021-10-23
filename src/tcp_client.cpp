//
// Created by wlbf on 10/18/21.
//

#include "event_loop.h"
#include "tcp_client.h"
#include "connector.h"
#include "socket_ops.h"
#include <glog/logging.h>
#include <cstdio>

namespace rift {
    namespace detail {

        void RemoveConnection(EventLoop *loop, const TcpConnectionPtr &conn) {
            loop->QueueInLoop([conn] { conn->ConnectDestroyed(); });
        }

        void RemoveConnector(const ConnectorPtr &connector) {
            //connector->
        }

    }

    TcpClient::TcpClient(EventLoop *loop, InetAddress &server_addr)
            : loop_(loop),
              connector_(new Connector(loop, server_addr)),
              retry_(false),
              connect_(true),
              next_conn_id_(1) {
        connector_->SetNewConnectionCallback([this](SocketPtr &&sock) { NewConnection(std::move(sock)); });

        // FIXME setConnectFailedCallback
        VLOG(0) << "TcpClient::TcpClient[" << this
                << "] - Connector " << connector_.get();
    }

    TcpClient::~TcpClient() {
        VLOG(0) << "TcpClient::~TcpClient[" << this
                << "] - Connector " << connector_.get();
        TcpConnectionPtr conn;
        {
            std::scoped_lock lock(mutex_);
            conn = connection_;
        }

        if (conn) {
            // FIXME: not 100% safe, if we are in different thread
            CloseCallback cb = [loop = loop_](const TcpConnectionPtr &conn) { detail::RemoveConnection(loop, conn); };
            loop_->RunInLoop([conn, cb] { conn->SetCloseCallback(cb); });
        } else {
            connector_->Stop();
            // FIXME: HACK
            loop_->RunAfter(1, [connector = connector_] { detail::RemoveConnector(connector); });
        }

    }

    void TcpClient::Connect() {
        // FIXME: check state
        VLOG(0) << "TcpClient::Connect[" << this << "] - connecting to "
                << connector_->ServerAddress().ToHostPort();
        connect_ = true;
        connector_->Start();
    }

    void TcpClient::Disconnect() {
        connect_ = false;
        {
            std::scoped_lock lock(mutex_);
            if (connection_) {
                connection_->Shutdown();
            }
        }
    }

    void TcpClient::Stop() {
        connect_ = false;
        connector_->Stop();
    }

    void TcpClient::NewConnection(SocketPtr &&sock) {
        loop_->AssetInLoopThread();
        InetAddress peer_addr(sockets::GetPeerAddr(sock->Fd()));
        InetAddress local_addr(sockets::GetLocalAddr(sock->Fd()));
        char buf[32];
        snprintf(buf, sizeof buf, ":%s#%d", peer_addr.ToHostPort().c_str(), next_conn_id_);
        next_conn_id_++;
        std::string conn_name = buf;

        // FIXME poll with zero timeout to double confirm the new connection
        // FIXME use make_shared if necessary
        TcpConnectionPtr conn(new TcpConnection(loop_, conn_name, std::move(sock), local_addr, peer_addr));
        conn->SetConnectionCallback(connection_callback_);
        conn->SetMessageCallback(message_callback_);
        conn->SetWriteCompleteCallback(write_complete_callback_);

        // FIXME: unsafe
        conn->SetCloseCallback([this](const TcpConnectionPtr &conn) { RemoveConnection(conn); });
        {
            std::scoped_lock lock(mutex_);
            connection_ = conn;
        }
        conn->ConnectEstablished();
    }

    void TcpClient::RemoveConnection(const TcpConnectionPtr &conn) {
        loop_->AssetInLoopThread();
        assert(loop_ == conn->GetLoop());

        {
            std::scoped_lock lock(mutex_);
            assert(connection_ == conn);
            connection_.reset();
        }

        loop_->QueueInLoop([conn] { conn->ConnectDestroyed(); });
        if (retry_ && connect_) {
            VLOG(0) << "TcpClient::Connect[" << this << "] - Reconnecting to "
                    << connector_->ServerAddress().ToHostPort();
            connector_->Restart();
        }
    }
}

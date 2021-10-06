//
// Created by wlbf on 9/10/21.
//

#include "socket.h"
#include "socket_ops.h"
#include "inet_address.h"
#include <glog/logging.h>
#include <netinet/in.h>
#include <strings.h>  // bzero

namespace rift {

    Socket::~Socket() {
        sockets::Close(sock_fd_);
    }

    void Socket::BindAddress(const InetAddress &local_addr) const {
        sockets::BindOrDie(sock_fd_, local_addr.GetSockAddrInet());
    }

    void Socket::Listen() const {
        sockets::ListenOrDie(sock_fd_);
    }

    int Socket::Accept(InetAddress *peer_addr) const {
        struct sockaddr_in addr{};
        bzero(&addr, sizeof addr);
        int conn_fd = sockets::Accept(sock_fd_, &addr);
        if (conn_fd >= 0) {
            peer_addr->SetSockAddrInet(addr);
        }
        return conn_fd;
    }

    void Socket::SetReuseAddr(bool on) const {
        int opt_val = on ? 1 : 0;
        int ret = ::setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);
        if (ret < 0) {
            LOG(FATAL) << "socket::SetReuseAddr";
        }
    }
}

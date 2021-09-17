//
// Created by wlbf on 9/10/21.
//

#include "socket_ops.h"
#include "glog/logging.h"
#include "fcntl.h"
#include <cerrno>
#include <cstdio>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {
    using SA = struct sockaddr;

    const SA *sockaddr_cast(const struct sockaddr_in *addr) {
        return reinterpret_cast<const SA *>(addr);
    }

    SA *sockaddr_cast(struct sockaddr_in *addr) {
        return reinterpret_cast<SA *>(addr);
    }

    void SetNonBlockAndCloseOnExec(int sockfd) {
        // non-block
        int flags = ::fcntl(sockfd, F_GETFL, 0);
        flags |= O_NONBLOCK;
        int ret = ::fcntl(sockfd, F_SETFL, flags);
        if (ret < 0) {
            abort();
        }

        // close-on-exec
        flags = ::fcntl(sockfd, F_GETFD, 0);
        flags |= FD_CLOEXEC;
        ret = ::fcntl(sockfd, F_SETFD, flags);
        if (ret < 0) {
            abort();
        }
    }
}


namespace rift::sockets {
    int CreateNonblockingOrDie() {
#if VALGRIND
        int sock_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock_fd < 0) {
            LOG(ERROR) << "sockets::CreateNonblockingOrDie";
        }
        SetNonBlockAndCloseOnExec(sock_fd);
#else
        int sock_fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
        if (sock_fd < 0) {
            LOG(ERROR) << "sockets::CreateNonblockingOrDie";
            abort();
        }
#endif
        return sock_fd;
    }

    void BindOrDie(int sock_fd, const struct sockaddr_in &addr) {
        int ret = ::bind(sock_fd, sockaddr_cast(&addr), sizeof addr);
        if (ret < 0) {
            LOG(ERROR) << "sockets::BindOrDie";
            abort();
        }
    }

    void ListenOrDie(int sock_fd) {
        int ret = ::listen(sock_fd, SOMAXCONN);
        if (ret < 0) {
            LOG(ERROR) << "sockets::ListenOrDie";
            abort();
        }
    }

    int Accept(int sock_fd, struct sockaddr_in *addr) {

        socklen_t addrlen = sizeof *addr;
#if VALGRIND
        int conn_fd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
        setNonBlockAndCloseOnExec(connfd);
#else
        int conn_fd = ::accept4(sock_fd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
        if (conn_fd < 0) {
            int savedErrno = errno;
            LOG(ERROR) << "Socket::Accept";
            switch (savedErrno) {
                case EAGAIN:
                case ECONNABORTED:
                case EINTR:
                case EPROTO: // ???
                case EPERM:
                case EMFILE: // per-process lmit of open file desctiptor ???
                    // expected errors
                    errno = savedErrno;
                    break;
                case EBADF:
                case EFAULT:
                case EINVAL:
                case ENFILE:
                case ENOBUFS:
                case ENOMEM:
                case ENOTSOCK:
                case EOPNOTSUPP:
                    // unexpected errors
                    LOG(ERROR) << "unexpected error of ::accept " << savedErrno;
                    abort();
                default:
                    LOG(ERROR) << "unknown error of ::accept " << savedErrno;;
                    abort();
            }
        }
        return conn_fd;
    }

    void Close(int sock_fd) {
        if (::close(sock_fd) < 0) {
            LOG(ERROR) << "sockets::Close";
            abort();
        }
    }

    void ToHostPort(char *buf, size_t size, const struct sockaddr_in &addr) {
        char host[INET_ADDRSTRLEN] = "INVALID";
        ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof host);
        uint16_t port = sockets::NetworkToHost16(addr.sin_port);
        snprintf(buf, size, "%s:%u", host, port);
    }

    void FromHostPort(const char *ip, uint16_t port, struct sockaddr_in *addr) {
        addr->sin_family = AF_INET;
        addr->sin_port = HostToNetwork16(port);
        if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
            LOG(ERROR) << "sockets::FromHostPort";
            abort();
        }
    }
}
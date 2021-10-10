//
// Created by wlbf on 9/10/21.
//

#ifndef RIFT_SOCKET_H
#define RIFT_SOCKET_H

#include <memory>

namespace rift {
    class InetAddress;

    ///
    /// Wrapper of socket file descriptor.
    ///
    /// It closes the sockfd when desctructs.
    /// It's thread safe, all operations are delagated to OS.
    class Socket {
    public:
        explicit Socket(int sock_fd) : sock_fd_(sock_fd) {}

        Socket(const Socket &) = delete;

        Socket &operator=(const Socket &) = delete;

        ~Socket();

        [[nodiscard]] int Fd() const { return sock_fd_; }

        /// abort if address in use
        void BindAddress(const InetAddress &local_addr) const;

        /// abort if address in use
        void Listen() const;


        /// On success, returns a non-negative integer that is
        /// a descriptor for the accepted socket, which has been
        /// set to non-blocking and close-on-exec. *peeraddr is assigned.
        /// On error, -1 is returned, and *peeraddr is untouched.
        int Accept(InetAddress *peer_addr) const;

        ///
        /// Enable/disable SO_REUSEADDR
        ///
        void SetReuseAddr(bool on) const;

        void ShutdownWrite() const;

        ///
        /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
        ///
        void SetTcpNoDelay(bool on) const;

    private:
        const int sock_fd_;
    };

    using SocketPtr = std::shared_ptr<Socket>;
}

#endif //RIFT_SOCKET_H

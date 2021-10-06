//
// Created by wlbf on 9/10/21.
//

#ifndef RIFT_INET_ADDRESS_H
#define RIFT_INET_ADDRESS_H

#include <string>
#include <netinet/in.h>

namespace rift {
    ///
    /// Wrapper of sockaddr_in.
    ///
    /// This is an POD interface class.
    class InetAddress {
    public:
        /// Construct an endpoint with given port number.
        /// Mostly used in TcpServer listening.
        explicit InetAddress(uint16_t port);

        /// Constructs an endpoint with given ip an port.
        /// @c ip should be "1.2.3.4"
        InetAddress(const std::string &ip, uint16_t port);

        /// Constructs an endpoint with given struct @c sockaddr_in.
        /// Mostly used when accepting new connections.
        explicit InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}

        [[nodiscard]] std::string ToHostPort() const;

        // default copy/assignment are okay.

        [[nodiscard]] const struct sockaddr_in &GetSockAddrInet() const { return addr_; }

        void SetSockAddrInet(const struct sockaddr_in &addr) { addr_ = addr; }

    private:
        struct sockaddr_in addr_{};
    };
}

#endif //RIFT_INET_ADDRESS_H

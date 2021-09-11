//
// Created by wlbf on 9/10/21.
//

#include "inet_address.h"
#include "socket_ops.h"
#include <strings.h>

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

namespace rift {
    static const in_addr_t k_inaddr_any = INADDR_ANY;

    static_assert(sizeof(InetAddress) == sizeof(sockaddr_in));

    InetAddress::InetAddress(uint16_t port) {
        bzero(&addr_, sizeof addr_);
        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = sockets::HostToNetwork32(k_inaddr_any);
        addr_.sin_port = sockets::HostToNetwork16(port);
    }

    InetAddress::InetAddress(const std::string &ip, uint16_t port) {
        bzero(&addr_, sizeof addr_);
        sockets::FromHostPort(ip.c_str(), port, &addr_);
    }

    std::string InetAddress::ToHostPort() const {
        char buf[32];
        sockets::ToHostPort(buf, sizeof buf, addr_);
        return buf;
    }
}


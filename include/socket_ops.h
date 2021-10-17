//
// Created by wlbf on 9/10/21.
//

#ifndef RIFT_SOCKET_OPS_H
#define RIFT_SOCKET_OPS_H


#include <arpa/inet.h>
#include <endian.h>

namespace rift::sockets {

    inline uint64_t HostToNetwork64(uint64_t host64) {
        return htobe64(host64);
    }

    inline uint32_t HostToNetwork32(uint32_t host32) {
        return htonl(host32);
    }

    inline uint16_t HostToNetwork16(uint16_t host16) {
        return htons(host16);
    }

    inline uint64_t NetworkToHost64(uint64_t net64) {
        return be64toh(net64);
    }

    inline uint32_t NetworkToHost32(uint32_t net32) {
        return ntohl(net32);
    }

    inline uint16_t NetworkToHost16(uint16_t net16) {
        return ntohs(net16);
    }

    ///
    /// Creates a non-blocking socket file descriptor,
    /// abort if any error.
    int CreateNonblockingOrDie();

    int  Connect(int sock_fd, const struct sockaddr_in& addr);

    void BindOrDie(int sock_fd, const struct sockaddr_in &addr);

    void ListenOrDie(int sock_fd);

    int Accept(int sock_fd, struct sockaddr_in *addr);

    void Close(int sock_fd);

    void ToHostPort(char *buf, size_t size, const struct sockaddr_in &addr);

    void FromHostPort(const char *ip, uint16_t port, struct sockaddr_in *addr);

    struct sockaddr_in GetLocalAddr(int sock_fd);
    struct sockaddr_in GetPeerAddr(int sock_fd);

    int GetSocketError(int sock_fd);

    void ShutdownWrite(int sock_fd);

    bool IsSelfConnect(int sock_fd);
}

#endif //RIFT_SOCKET_OPS_H

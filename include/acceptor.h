//
// Created by wlbf on 9/10/21.
//

#ifndef RIFT_ACCEPTOR_CC_H
#define RIFT_ACCEPTOR_CC_H

#include "channel.h"
#include "socket.h"

namespace rift {
    class EventLoop;

    class InetAddress;

    ///
    /// Acceptor of incoming TCP connections;
    ///

    class Acceptor {
    public:
        using NewConnectionCallback = std::function<void(Socket &&, const InetAddress &)>;

        Acceptor(EventLoop *loop, const InetAddress &listen_addr);

        Acceptor(const Acceptor &) = delete;

        Acceptor &operator=(const Acceptor &) = delete;

        void SetNewConnectionCallback(const NewConnectionCallback &cb) { new_connection_callback_ = cb; }

        [[nodiscard]] bool Listening() const { return listening_; }

        void Listen();

    private:
        void HandleRead();

        EventLoop *loop_;
        Socket accept_socket_;
        Channel accept_channel_;
        NewConnectionCallback new_connection_callback_;
        bool listening_;
    };
}

#endif //RIFT_ACCEPTOR_CC_H

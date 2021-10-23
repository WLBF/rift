//
// Created by wlbf on 10/14/21.
//

#ifndef RIFT_CONNECTOR_H
#define RIFT_CONNECTOR_H

#include "inet_address.h"
#include "timer_id.h"
#include "socket.h"
#include <functional>
#include <memory>

namespace rift {

    class Channel;

    class EventLoop;

    class Connector {
    public:
        using NewConnectionCallback = std::function<void(SocketPtr &&sock)>;

        Connector(EventLoop *loop, InetAddress &server_addr);

        ~Connector();

        void SetNewConnectionCallback(const NewConnectionCallback &cb) {
            new_connection_callback_ = cb;
        }

        void Start();  // can be called in any thread
        void Restart();  // must be called in loop thread
        void Stop();  // can be called in any thread

        [[nodiscard]] const InetAddress &ServerAddress() const { return server_addr_; }

    private:
        enum States {
            k_disconnected, k_connecting, k_connected
        };
        static const int k_max_retry_delay_ms = 30 * 1000;
        static const int k_init_retry_delay_ms = 500;

        void SetState(States s) { state_ = s; }

        void StartInLoop();

        void Connect();

        void Connecting(int sock_fd);

        void HandleWrite();

        void HandleError();

        void Retry(int sock_fd);

        int RemoveAndResetChannel();

        void ResetChannel();

        EventLoop *loop_;
        InetAddress server_addr_;
        bool connect_; // atomic
        States state_;  // FIXME: use atomic variable
        std::unique_ptr<Channel> channel_;
        NewConnectionCallback new_connection_callback_;
        int retry_delay_ms_;
        TimerID timer_id_;
    };

    using ConnectorPtr = std::shared_ptr<Connector>;

}
#endif //RIFT_CONNECTOR_H

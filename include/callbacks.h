//
// Created by wlbf on 9/4/21.
//

#ifndef RIFT_CALLBACKS_H
#define RIFT_CALLBACKS_H

#include <functional>
#include <memory>

namespace rift {
    // All client visible callbacks go here.

    class TcpConnection;

    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

    using TimerCallback = std::function<void()>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
    using MessageCallback = std::function<void(const TcpConnectionPtr &, const char *data, ssize_t len)>;
    using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
}

#endif //RIFT_CALLBACKS_H

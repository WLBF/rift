//
// Created by wlbf on 8/31/21.
//

#ifndef RIFT_CHANNEL_H
#define RIFT_CHANNEL_H

#include <functional>
#include "time_point.h"

namespace rift {

    class EventLoop;

    ///
    /// A selectable I/O channel.
    ///
    /// This class doesn't own the file descriptor.
    /// The file descriptor could be a socket,
    /// an eventfd, a timerfd, or a signalfd
    class Channel {

    public:
        using EventCallback = std::function<void()>;
        using ReadEventCallback = std::function<void(time::TimePoint)>;

        Channel(EventLoop *loop, int fd);

        Channel(const Channel &) = delete;

        Channel &operator=(const Channel &) = delete;

        ~Channel();

        void HandleEvent(time::TimePoint receive_time);

        void SetReadCallback(const ReadEventCallback &cb) { read_callback_ = cb; }

        void SetWriteCallback(const EventCallback &cb) { write_callback_ = cb; }

        void SetErrorCallback(const EventCallback &cb) { error_callback_ = cb; }

        void SetCloseCallback(const EventCallback &cb) { close_callback_ = cb; }

        [[nodiscard]] int Fd() const { return fd_; }

        [[nodiscard]] int Events() const { return events_; };

        void SetEvents(int revt) { revents_ = revt; }

        [[nodiscard]] bool IsNoneEvent() const { return events_ == k_none_event; }

        void EnableReading() {
            events_ |= k_read_event;
            Update();
        }

        void DisableAll() {
            events_ = k_none_event;
            Update();
        }

        // for Poller
        [[nodiscard]] int Index() const { return index_; }

        void SetIndex(int idx) { index_ = idx; }

        EventLoop *OwnerLoop() { return loop_; }

    private:

        void Update();

        static const int k_none_event;
        static const int k_read_event;
        static const int k_write_event;

        EventLoop *loop_;
        const int fd_;
        int events_;
        int revents_;
        int index_;

        bool event_handling_;

        ReadEventCallback read_callback_;
        EventCallback write_callback_;
        EventCallback error_callback_;
        EventCallback close_callback_;
    };


}

#endif //RIFT_CHANNEL_H

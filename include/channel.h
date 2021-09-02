//
// Created by wlbf on 8/31/21.
//

#ifndef RIFT_CHANNEL_H
#define RIFT_CHANNEL_H

#include <functional>

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

        Channel(EventLoop *loop, int fd);

        Channel(const Channel &) = delete;

        Channel &operator=(const Channel &) = delete;

        void HandleEvent();

        void SetReadCallback(const EventCallback &cb) { read_callback_ = cb; }

        void SetWriteCallback(const EventCallback &cb) { write_callback_ = cb; }

        void SetErrorCallback(const EventCallback &cb) { error_callback_ = cb; }

        int Fd() const { return fd_; }

        int Events() const { return events_; };

        void SetEvents(int revt) { revents_ = revt; }

        bool IsNoneEvent() const { return events_ == k_none_event; }

        void EnableReading() {
            events_ |= k_read_event;
            Update();
        }

        // for Poller
        int Index() const { return index_; }

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

        EventCallback read_callback_;
        EventCallback write_callback_;
        EventCallback error_callback_;
    };


}

#endif //RIFT_CHANNEL_H
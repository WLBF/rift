//
// Created by wlbf on 10/25/21.
//

#ifndef RIFT_EPOLLER_H
#define RIFT_EPOLLER_H

#include "event_loop.h"
#include "time_point.h"
#include <vector>
#include <map>

struct epoll_event;

namespace rift {
    class Channel;

    ///
    /// IO Multiplexing with epoll(4).
    ///
    /// This class doesn't own the Channel objects.
    class EPoller {
    public:
        using ChannelList = std::vector<Channel *>;

        explicit EPoller(EventLoop *loop);

        ~EPoller();

        /// Polls the I/O events.
        /// Must be called in the loop thread.
        time::TimePoint Poll(int timeout_ms, ChannelList *active_channels);

        /// Changes the interested I/O events.
        /// Must be called in the loop thread.
        void UpdateChannel(Channel *channel);

        /// Remove the channel, when it destructs.
        /// Must be called in the loop thread.
        void RemoveChannel(Channel *channel);

    private:
        static const int k_init_event_list_size = 16;

        void FillActiveChannels(int event_num, ChannelList *active_channels) const;

        void AssertInLoopThread() { owner_loop_->AssetInLoopThread(); }

        void Update(int operation, Channel *channel) const;

        using EventList = std::vector<struct epoll_event>;
        using ChannelMap = std::map<int, Channel *>;

        EventLoop *owner_loop_;
        int epoll_fd_;
        EventList events_;
        ChannelMap channels_;
    };
}


#endif //RIFT_EPOLLER_H

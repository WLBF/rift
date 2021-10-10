//
// Created by wlbf on 8/31/21.
//

#ifndef RIFT_POLLER_H
#define RIFT_POLLER_H

#endif //RIFT_POLLER_H

#include "event_loop.h"
#include "channel.h"
#include "time_point.h"

#include <map>
#include <vector>

struct pollfd;

namespace rift {
    class Poller;

    ///
    /// IO Multiplexing with poll(2).
    ///
    /// This class doesn't own the Channel objects.
    class Poller {
    public:
        using ChannelList = std::vector<Channel *>;

        explicit Poller(EventLoop *loop);

        Poller(const Poller &) = delete;

        Poller &operator=(const Poller &) = delete;

        ~Poller();

        /// Polls the I/O events.
        /// Must be called in the loop thread
        time::TimePoint Poll(int timeout_ms, ChannelList *active_channels);

        /// Changes the interested I/O events.
        /// Must be called in the loop thread.
        void UpdateChannel(Channel *channel);

        /// Remove the channel, when it destructs.
        /// Must be called in the loop thread.
        void RemoveChannel(Channel *channel);


        void AssertInLoopThread() { owner_loop_->AssetInLoopThread(); }

    private:
        void FillActiveChannels(int num_events, ChannelList *active_channels) const;

        using PollFdList = std::vector<struct pollfd>;
        using ChannelMap = std::map<int, Channel *>;

        EventLoop *owner_loop_;
        PollFdList pollfds_;
        ChannelMap channels_;
    };

}

//
// Created by wlbf on 8/31/21.
//

#include "poller.h"
#include <poll.h>
#include <glog/logging.h>
#include <cassert>


namespace rift {

    Poller::Poller(EventLoop *loop) : owner_loop_(loop) {}

    Poller::~Poller() {}

    void Poller::Poll(int timeout_ms, Poller::ChannelList *active_channels) {
        int num_events = ::poll(pollfds_.data(), pollfds_.size(), timeout_ms);
        if (num_events > 0) {
            LOG(INFO) << num_events << "events happened";
            FillActiveChannels(num_events, active_channels);
        } else if (num_events == 0) {
            LOG(INFO) << "nothing happened";
        } else {
            LOG(ERROR) << "Poller::poll()";
        }
    }

    void Poller::FillActiveChannels(int num_events, Poller::ChannelList *active_channels) const {
        for (auto it = pollfds_.begin(); it != pollfds_.end() && num_events > 0; it++) {
            if (it->revents > 0) {
                num_events--;
                auto ch = channels_.find(it->fd);
                assert(ch != channels_.end());
                Channel *channel = ch->second;
                assert(channel->Fd() == it->fd);
                channel->SetEvents(it->revents);
                active_channels->push_back(channel);
            }
        }
    }

    void Poller::UpdateChannel(Channel *channel) {
        AssertInLoopThread();
        LOG(INFO) << "fd = " << channel->Fd() << " events = " << channel->Events();
        if (channel->Index() < 0) {
            // a new one, add to pollfds_
            assert(channels_.find(channel->Fd()) == channels_.end());
            struct pollfd pfd{};
            pfd.fd = channel->Fd();
            pfd.events = static_cast<short>(channel->Events());
            pfd.revents = 0;
            pollfds_.push_back(pfd);
            int idx = static_cast<int>(pollfds_.size()) - 1;
            channel->SetIndex(idx);
            channels_[pfd.fd] = channel;
        } else {
            // update existing one
            assert(channels_.find(channel->Fd()) != channels_.end());
            assert(channels_[channel->Fd()] == channel);
            int idx = channel->Index();
            assert(idx >= 0 && idx < pollfds_.size());
            auto &pfd = pollfds_[idx];
            assert(pfd.fd == channel->Fd() || pfd.fd == -1);
            pfd.events = static_cast<short>(channel->Events());
            pfd.revents = 0;
            if (channel->IsNoneEvent()) {
                // ignore this pollfd
                pfd.fd = -1;
            }
        }
    }
}


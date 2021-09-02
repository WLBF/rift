//
// Created by wlbf on 8/31/21.
//

#include "channel.h"
#include "event_loop.h"

#include <poll.h>
#include <glog/logging.h>

namespace rift {
    const int Channel::k_none_event = 0;
    const int Channel::k_read_event = POLLIN | POLLPRI;
    const int Channel::k_write_event = POLLOUT;

    Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1) {}

    void Channel::HandleEvent() {

        if (revents_ & POLLNVAL) {
            LOG(WARNING) << "Channel::HandleEvent() POLLNVAL";
        }

        if (revents_ & (POLLERR | POLLNVAL)) {
            if (error_callback_) error_callback_();
        }
        if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
            if (read_callback_) read_callback_();
        }
        if (revents_ & POLLOUT) {
            if (write_callback_) write_callback_();
        }
    }

    void Channel::Update() {
        loop_->UpdateChannel(this);
    }
}


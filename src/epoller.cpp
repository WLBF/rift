//
// Created by wlbf on 10/25/21.
//

#include "epoller.h"
#include "channel.h"

#include <cassert>
#include <cerrno>
#include <poll.h>
#include <sys/epoll.h>
#include <glog/logging.h>

using namespace rift;

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN);
static_assert(EPOLLPRI == POLLPRI);
static_assert(EPOLLOUT == POLLOUT);
static_assert(EPOLLRDHUP == POLLRDHUP);
static_assert(EPOLLERR == POLLERR);
static_assert(EPOLLHUP == POLLHUP);


namespace {
    const int k_new = -1;
    const int k_added = 1;
    const int k_deleted = 2;
}

EPoller::EPoller(EventLoop *loop)
        : owner_loop_(loop),
          epoll_fd_(::epoll_create(EPOLL_CLOEXEC)),
          events_(k_init_event_list_size) {
    if (epoll_fd_ < 0) {
        LOG(FATAL) << "EPoller::EPoller";
    }
}

EPoller::~EPoller() {
    ::close(epoll_fd_);
}

time::TimePoint EPoller::Poll(int timeout_ms, EPoller::ChannelList *active_channels) {
    int event_num = ::epoll_wait(epoll_fd_, events_.data(), static_cast<int>(events_.size()), timeout_ms);
    auto now = time::Now();
    if (event_num > 0) {
        VLOG(5) << event_num << " events happened";
        FillActiveChannels(event_num, active_channels);
        if (event_num == events_.size()) {
            events_.resize(events_.size() * 2);
        } else if (event_num == 0) {
            VLOG(5) << "nothing happened";
        } else {
            LOG(ERROR) << "EPoller::Poll()";
        }
    }
    return now;
}

void EPoller::FillActiveChannels(int event_num, EPoller::ChannelList *active_channels) const {
    assert(event_num <= events_.size());
    for (int i = 0; i < event_num; i++) {
        auto *channel = static_cast<Channel *>(events_[i].data.ptr);
#if NDEBUG
        int fd = channel->Fd();
        auto it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
#endif
        channel->SetRevents(static_cast<int>(events_[i].events));
        active_channels->push_back(channel);
    }
}

void EPoller::UpdateChannel(Channel *channel) {
    AssertInLoopThread();
    VLOG(5) << "fd = " << channel->Fd() << " events = " << channel->Events();
    const int index = channel->Index();
    if (index == k_new || index == k_deleted) {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->Fd();
        if (index == k_new) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        } else {
            // index == k_deleted
            assert(channels_.find(fd) != channels_.end());
            channels_[fd] = channel;
        }
        channel->SetIndex(k_added);
        Update(EPOLL_CTL_ADD, channel);
    } else {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->Fd();
        assert(channels_.find(fd) != channels_.end());
        channels_[fd] = channel;
        assert(index == k_added);
        if (channel->IsNoneEvent()) {
            Update(EPOLL_CTL_DEL, channel);
            channel->SetIndex(k_deleted);
        } else {
            Update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPoller::RemoveChannel(Channel *channel) {
    AssertInLoopThread();
    int fd = channel->Fd();
    VLOG(5) << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    channels_[fd] = channel;
    assert(channel->IsNoneEvent());
    int index = channel->Index();
    assert(index == k_added || index == k_deleted);
    size_t n = channels_.erase(fd);
    assert(n == 1);

    if (index == k_added) {
        Update(EPOLL_CTL_DEL, channel);
    }
    channel->SetIndex(k_new);
}

void EPoller::Update(int operation, Channel *channel) const {
    struct epoll_event event{};
    bzero(&event, sizeof event);
    event.events = channel->Events();
    event.data.ptr = channel;
    int fd = channel->Fd();
    if (::epoll_ctl(epoll_fd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG(ERROR) << "epoll_ctl op=" << operation << " fd=" << fd;
        } else {
            LOG(FATAL) << "epoll_ctl op=" << operation << " fd=" << fd;
        }
    }
}

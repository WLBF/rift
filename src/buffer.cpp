//
// Created by wlbf on 10/7/21.
//

#include "buffer.h"

#include <memory.h>
#include <sys/uio.h>

namespace rift {
    ssize_t Buffer::ReadFd(int fd, int *saved_errno) {
        char extra_buf[65536];
        struct iovec vec[2];
        const size_t writable = WritableBytes();
        vec[0].iov_base = Begin() + write_index_;
        vec[0].iov_len = writable;
        vec[1].iov_base = extra_buf;
        vec[1].iov_len = sizeof extra_buf;
        const ssize_t n = readv(fd, vec, 2);
        if (n < 0) {
            *saved_errno = errno;
        } else if (n < writable) {
            write_index_ += n;
        } else {
            write_index_ = buffer_.size();
            Append(extra_buf, n - writable);
        }
        return n;
    }
}
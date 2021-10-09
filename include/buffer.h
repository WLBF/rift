//
// Created by wlbf on 10/7/21.
//

#ifndef RIFT_BUFFER_H
#define RIFT_BUFFER_H

#include <vector>
#include <string>
#include <cassert>

namespace rift {

    /// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
    ///
    /// @code
    /// +-------------------+------------------+------------------+
    /// | prependable bytes |  readable bytes  |  writable bytes  |
    /// |                   |     (CONTENT)    |                  |
    /// +-------------------+------------------+------------------+
    /// |                   |                  |                  |
    /// 0      <=      readerIndex   <=   writerIndex    <=     size
    /// @endcode
    class Buffer {
    public:
        static const size_t k_cheap_prepend = 8;
        static const size_t k_initial_size = 1024;

        Buffer()
                : buffer_(k_cheap_prepend + k_initial_size),
                  read_index_(k_cheap_prepend),
                  write_index_(k_cheap_prepend) {
            assert(ReadableBytes() == 0);
            assert(WritableBytes() == k_initial_size);
            assert(PrependableBytes() == k_cheap_prepend);
        }


        [[nodiscard]] size_t ReadableBytes() const {
            return write_index_ - read_index_;
        }

        size_t WritableBytes() {
            return buffer_.size() - write_index_;
        }

        [[nodiscard]] size_t PrependableBytes() const {
            return read_index_;
        }

        void Swap(Buffer &rhs) {
            buffer_.swap(rhs.buffer_);
            std::swap(read_index_, rhs.read_index_);
            std::swap(write_index_, rhs.write_index_);
        }

        [[nodiscard]] const char *Peek() const {
            return Begin() + read_index_;
        }

        // retrieve returns void, to prevent
        // string str(retrieve(readableBytes()), readableBytes());
        // the evaluation of two functions are unspecified
        void Retrieve(size_t len) {
            assert(len < ReadableBytes());
            read_index_ += len;
        }

        void RetrieveUntil(const char *end) {
            assert(Peek() <= end);
            assert(end <= BeginWrite());
        }

        void RetrieveAll() {
            read_index_ = k_cheap_prepend;
            write_index_ = k_cheap_prepend;
        }

        std::string RetrieveAsString() {
            std::string str(Peek(), ReadableBytes());
            RetrieveAll();
            return str;
        }

        void Append(const std::string &str) {
            Append(str.data(), str.length());
        }

        void Append(const char * /*restrict*/ data, size_t len) {
            EnsureWritableBytes(len);
            std::copy(data, data + len, BeginWrite());
            HasWritten(len);
        }

        void Append(const void * /*restrict*/ data, size_t len) {
            Append(static_cast<const char *>(data), len);
        }

        void EnsureWritableBytes(size_t len) {
            if (WritableBytes() < len) {
                MakeSpace(len);
            }
            assert(WritableBytes() >= len);
        }

        char *BeginWrite() {
            return Begin() + write_index_;
        }

        void HasWritten(size_t len) {
            write_index_ += len;
        }

        void Prepend(const void * /*restrict*/ data, size_t len) {
            assert(len <= PrependableBytes());
            read_index_ -= len;
            const char *d = static_cast<const char *>(data);
            std::copy(d, d + len, Begin() + read_index_);
        }

        /// Read data directly into buffer.
        ///
        /// It may implement with readv(2)
        /// @return result of read(2), @c errno is saved
        ssize_t ReadFd(int fd, int *savedErrno);

    private:
        [[nodiscard]] char *Begin() const {
            return const_cast<char *>(&*buffer_.begin());
        }

        void MakeSpace(size_t len) {
            if (WritableBytes() + PrependableBytes() < len + k_cheap_prepend) {
                buffer_.resize(write_index_ + len);
            } else {
                assert(read_index_ > k_cheap_prepend);
                size_t readable = ReadableBytes();
                std::copy(Begin() + read_index_, Begin() + write_index_, Begin() + k_cheap_prepend);
                read_index_ = k_cheap_prepend;
                write_index_ = read_index_ + readable;
                assert(readable == ReadableBytes());
            }
        }

        std::vector<char> buffer_;
        size_t read_index_;
        size_t write_index_;
    };
}

#endif //RIFT_BUFFER_H

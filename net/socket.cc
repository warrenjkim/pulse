#include "net/socket.h"

#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <string>
#include <string_view>

namespace pulse::net {

Socket::Socket(int fd) : fd_(fd) {}

Socket::~Socket() {
  if (ok()) {
    close(fd_);
  }
}

Socket::Socket(Socket&& other) : fd_(other.fd_) { other.fd_ = -1; }

Socket& Socket::operator=(Socket&& other) {
  if (this != &other) {
    fd_ = other.fd_;
    other.fd_ = -1;
  }

  return *this;
}

std::string Socket::read(size_t size) {
  std::string buffer(size, '\0');
  size_t actual = 0;
  while (actual < size) {
    ssize_t bytes =
        recv(fd_, buffer.data() + actual, size - actual, /*flags=*/0);
    if (bytes == 0) {
      break;
    }

    if (bytes < 0) {
      std::cerr << "[" << __FILE__ << ":" << __LINE__
                << "] recv() failed: " << strerror(errno) << "\n";
      return "";
    }

    actual += static_cast<size_t>(bytes);
  }

  buffer.resize(actual);
  return buffer;
}

std::string Socket::read_until(std::string_view delimiter, size_t max_bytes) {
  std::string buffer;
  char c;
  while (buffer.size() < max_bytes) {
    ssize_t bytes = recv(fd_, &c, /*size=*/1, /*flags=*/0);
    if (bytes == 0) {
      break;
    }

    if (bytes < 0) {
      std::cerr << "[" << __FILE__ << ":" << __LINE__
                << "] recv() failed: " << strerror(errno) << "\n";
      return "";
    }

    buffer += c;
    if (buffer.size() >= delimiter.size() && buffer.ends_with(delimiter)) {
      break;
    }
  }

  return buffer;
}

size_t Socket::write(std::string_view data) {
  size_t total = 0;
  while (total < data.size()) {
    ssize_t bytes = send(fd_, data.data() + total, data.size() - total, 0);
    if (bytes < 0) {
      std::cerr << "[" << __FILE__ << ":" << __LINE__
                << "] send() failed: " << strerror(errno) << "\n";
      return total;
    }

    total += static_cast<size_t>(bytes);
  }

  return total;
}

bool Socket::ok() const { return fd_ >= 0; }

}  // namespace pulse::net

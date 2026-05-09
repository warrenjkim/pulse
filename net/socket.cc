#include "net/socket.h"

#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <string_view>

namespace pulse::net {

Socket::Socket(int fd) : fd_(fd) {}

Socket::~Socket() {
  if (fd_ >= 0) {
    close(fd_);
  }
}

Socket::Socket(Socket&& other) : fd_(other.fd_) { other.fd_ = -1; }

Socket& Socket::operator=(Socket&& other) {
  fd_ = other.fd_;
  other.fd_ = -1;

  return *this;
}

int Socket::fd() const { return fd_; }

std::string Socket::read_until(std::string_view delimiter, size_t max_bytes) {
  std::string buffer;
  char c;
  while (buffer.size() < max_bytes) {
    ssize_t bytes = recv(fd_, &c, /*size=*/1, /*flags=*/0);
    if (bytes == 0) {
      break;
    }

    if (bytes < 0) {
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
      return total;
    }

    total += static_cast<size_t>(bytes);
  }

  return total;
}

}  // namespace pulse::net

#pragma once

#include <unistd.h>

#include <string>
#include <string_view>

namespace pulse::net {

class Socket {
 public:
  explicit Socket(int fd);
  ~Socket();

  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;

  Socket(Socket&& other);

  int fd() const;

  std::string read_until(std::string_view delimiter,
                        size_t max_bytes = kMaxRequestBytes);

  size_t write(std::string_view data);

 private:
  static constexpr int kMaxRequestBytes = 8 * 1024;

  int fd_;
};

}  // namespace pulse::net

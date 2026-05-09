#pragma once

#include <unistd.h>

#include <string>
#include <string_view>

namespace pulse::net {

// RAII wrapper around a POSIX file descriptor.
//
// Ownership semantics:
// * Each `Socket` instance owns `fd_`, which is closed upon destruction.
// * Copying is disabled to prevent double closing.
// * Moving transfers ownership of `fd_` and sets `other.fd_` to -1 to indicate
// a moved-from `Socket`. The destructor is a no-op in this state.
class Socket {
 public:
  // Takes ownership of `fd`. `fd` must be a valid file descriptor returned by
  // `accept()` or `socket()`.
  explicit Socket(int fd);

  // Closes the socket if and only if `fd_` is not -1.
  ~Socket();

  // Not copyable
  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;

  // Movable
  Socket(Socket&& other);
  Socket& operator=(Socket&&);

  // Reads bytes from the socket into a buffer until either:
  //   (1) `delimiter` is found, in which case, return the buffer excluding the
  //   delimiter.
  //   (2) `max_bytes` bytes have been read without finding the delimiter, in
  //   which case, return the buffer of size `max_bytes`.
  // This call is blocking.
  //
  // Returns an empty string if reading failed. The caller should treat this as
  // a signal to close the connection.
  std::string read_until(std::string_view delimiter,
                         size_t max_bytes = kMaxRequestBytes);

  // Writes `data` to the socket. This call is blocking.
  //
  // Returns the number of bytes actually written. A return value less than
  // data.size() indicates an error mid-write.
  size_t write(std::string_view data);

 private:
  static constexpr int kMaxRequestBytes = 8 * 1024;

  int fd_;
};

}  // namespace pulse::net

#pragma once

#include <sys/socket.h>

#include "net/socket.h"

namespace pulse::net {

// Binds to a port and accepts incoming TCP connections.
//
// Ownership semantics:
// * Each `Acceptor` instance owns `fd_`, which is closed upon destruction.
// * Copying and moving are disabled since there should be exactly one
// `Acceptor` per port for the lifetime of the server.
class Acceptor {
 public:
  // Binds to `port` and begins listening for incoming connections. Crashes on
  // failure.
  Acceptor(int port);

  // Closes the socket if `fd_` is not -1. No-op if construction failed.
  ~Acceptor();

  // Not copyable or movable
  Acceptor(const Acceptor&) = delete;
  Acceptor& operator=(const Acceptor&) = delete;

  // Blocks until an incoming connection is available, then returns a `Socket`
  // owning that connection. The caller is responsible for validating, reading
  // from, and writing to the returned `Socket`.
  Socket accept();

 private:
  int fd_;
};

}  // namespace pulse::net

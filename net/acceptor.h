#pragma once

#include <sys/socket.h>

#include "net/socket.h"

namespace pulse::net {

class Acceptor {
 public:
  Acceptor(int port);

  ~Acceptor();

  Socket accept();

 private:
  int fd_;
};

}  // namespace pulse::net

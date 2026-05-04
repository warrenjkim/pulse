#include "net/acceptor.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <iostream>

#include "net/socket.h"

namespace pulse::net {

Acceptor::Acceptor(int port) : fd_(-1) {
  int fd = socket(/*domain=*/AF_INET, /*type=*/SOCK_STREAM, /*protocol=*/0);
  if (fd < 0) {
    std::cerr << "socket\n";
    return;
  }

  int opt = 1;
  if (setsockopt(fd, /*level=*/SOL_SOCKET, /*option_name=*/SO_REUSEADDR, &opt,
                 sizeof(opt)) < 0) {
    std::cerr << "setsockopt\n";
    close(fd);
    return;
  }

  sockaddr_in address{.sin_family = AF_INET,
                      .sin_port = htons(port),
                      .sin_addr = {.s_addr = INADDR_ANY}};
  if (bind(fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
    std::cerr << "bind\n";
    close(fd);
    return;
  }

  if (listen(fd, /*backlog=*/128) < 0) {
    std::cerr << "listen\n";
    close(fd);
    return;
  }

  fd_ = fd;
}

Acceptor::~Acceptor() {
  if (fd_ >= 0) {
    close(fd_);
  }
}

Socket Acceptor::accept() {
  int fd = ::accept(fd_, /*address=*/nullptr, /*address_len=*/nullptr);
  if (fd < 0) {
    // TODO(handle errors)
    std::cerr << "Acceptor::accept()\n";
  }

  return Socket(fd);
}

}  // namespace pulse::net

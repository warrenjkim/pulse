#include "pulse/net/acceptor.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>

#include "pulse/core/log.h"
#include "pulse/net/socket.h"

namespace pulse::net {

Acceptor::Acceptor(int port) : fd_(-1) {
  int fd = socket(/*domain=*/AF_INET, /*type=*/SOCK_STREAM, /*protocol=*/0);
  if (fd < 0) {
    Log() << "socket() failed on port " << port << ": " << strerror(errno);
    std::exit(1);
  }

  int opt = 1;
  if (setsockopt(fd, /*level=*/SOL_SOCKET, /*option_name=*/SO_REUSEADDR, &opt,
                 sizeof(opt)) < 0) {
    Log() << "setsockopt() failed: " << strerror(errno);
    close(fd);
    std::exit(1);
  }

  sockaddr_in address{.sin_family = AF_INET,
                      .sin_port = htons(port),
                      .sin_addr = {.s_addr = INADDR_ANY}};
  if (bind(fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
    Log() << "bind() failed on port " << port << ": " << strerror(errno);
    close(fd);
    std::exit(1);
  }

  if (listen(fd, /*backlog=*/128) < 0) {
    Log() << "listen() failed: " << strerror(errno);
    close(fd);
    std::exit(1);
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
    Log() << "accept() failed: " << strerror(errno);
    return Socket(-1);
  }

  return Socket(fd);
}

}  // namespace pulse::net

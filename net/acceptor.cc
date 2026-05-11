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
    std::cerr << "[" << __FILE__ << ":" << __LINE__
              << "] socket() failed on port " << port << ": " << strerror(errno)
              << "\n";
    std::exit(1);
  }

  int opt = 1;
  if (setsockopt(fd, /*level=*/SOL_SOCKET, /*option_name=*/SO_REUSEADDR, &opt,
                 sizeof(opt)) < 0) {
    std::cerr << "[" << __FILE__ << ":" << __LINE__
              << "] setsockopt() failed: " << strerror(errno) << "\n";
    close(fd);
    std::exit(1);
  }

  sockaddr_in address{.sin_family = AF_INET,
                      .sin_port = htons(port),
                      .sin_addr = {.s_addr = INADDR_ANY}};
  if (bind(fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
    std::cerr << "[" << __FILE__ << ":" << __LINE__
              << "] bind() failed on port " << port << ": " << strerror(errno)
              << "\n";
    close(fd);
    std::exit(1);
  }

  if (listen(fd, /*backlog=*/128) < 0) {
    std::cerr << "[" << __FILE__ << ":" << __LINE__
              << "] listen() failed: " << strerror(errno) << "\n";
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
    std::cerr << "[" << __FILE__ << ":" << __LINE__
              << "] accept() failed: " << strerror(errno) << "\n";
    return Socket(-1);
  }

  return Socket(fd);
}

}  // namespace pulse::net

#pragma once

#include <cstddef>

#include "pulse/concurrent/thread_pool.h"
#include "pulse/http/router.h"
#include "pulse/net/acceptor.h"

namespace pulse::http {

class Server {
 public:
  struct Options {
    int port;
    size_t threads;
  };

  explicit Server(Router router, const Options& opts);

  // Not copyable or movable
  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  // Starts the accept loop. Blocks until the process is killed.
  void run();

 private:
  Router router_;
  concurrent::ThreadPool pool_;
  net::Acceptor acceptor_;
};

}  // namespace pulse::http

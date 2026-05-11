#pragma once

#include <map>
#include <memory>
#include <unordered_map>

#include "concurrent/thread_pool.h"
#include "http/handler.h"
#include "http/method.h"
#include "net/acceptor.h"

namespace pulse::http {

class Server {
 public:
  struct Options {
    int port;
    size_t threads;
  };

  explicit Server(const Options& opts);

  // Not copyable or movable
  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  // Registers a handler for the given method and path.
  //
  // NOTE: Must not be called if `run()` has been called.
  void route(Method method, std::string path, std::unique_ptr<Handler> handler);

  // Starts the accept loop. Blocks until the process is killed.
  void run();

 private:
  using Router =
      std::map<Method,
               std::unordered_map<std::string, std::unique_ptr<Handler>>>;

  Router router_;
  concurrent::ThreadPool pool_;
  net::Acceptor acceptor_;
};

}  // namespace pulse::http

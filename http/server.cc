#include "http/server.h"

#include <iostream>
#include <map>
#include <memory>
#include <unordered_map>

#include "concurrent/thread_pool.h"
#include "dsa/result.h"
#include "http/handler.h"
#include "http/internal/transform.h"
#include "http/method.h"
#include "net/acceptor.h"
#include "net/socket.h"

namespace pulse::http {

Server::Server(const Server::Options& opts)
    : pool_(opts.threads), acceptor_(opts.port) {}

void Server::route(Method method, std::string path,
                   std::unique_ptr<Handler> handler) {
  router_[method][std::move(path)] = std::move(handler);
}

void Server::run() {
  while (true) {
    net::Socket s = acceptor_.accept();
    if (!s.ok()) {
      continue;
    }

    // TODO(use move-only functions)
    pool_.submit([this, socket = std::make_shared<net::Socket>(std::move(s))] {
      std::string header = socket->read_until("\r\n\r\n");
      std::cerr << "header: [" << header << "]\n";
      if (header.empty()) {
        std::cerr << "empty header\n";
        return;
      }

      Result<Request> request = parse_header(header);
      if (!request.ok()) {
        std::cerr << "parse error: " << request.error().message << "\n";
        socket->write(to_string(Response{.content_type = "text/html",
                                         .status = 400,
                                         .body = "<h1>400 Bad Request</h1>"}));
        return;
      }

      auto method = router_.find(request->method);
      if (method == router_.end()) {
        socket->write(to_string(Response{.content_type = "text/html",
                                         .status = 404,
                                         .body = "<h1>404 Not Found</h1>"}));
        return;
      }

      auto handler = method->second.find(request->path);
      if (handler == method->second.end()) {
        socket->write(to_string(Response{.content_type = "text/html",
                                         .status = 404,
                                         .body = "<h1>404 Not Found</h1>"}));
        return;
      }

      if (auto it = request->headers.find("Content-Length");
          it != request->headers.end()) {
        request->body = socket->read(std::stoul(it->second));
      }

      socket->write(to_string((*handler->second)(*request)));
    });
  }
}

}  // namespace pulse::http

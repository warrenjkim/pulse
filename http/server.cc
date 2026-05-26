#include "http/server.h"

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "concurrent/thread_pool.h"
#include "core/result.h"
#include "core/stringify.h"
#include "http/handler.h"
#include "http/internal/transform.h"
#include "http/request.h"
#include "http/response.h"
#include "http/router.h"
#include "net/acceptor.h"
#include "net/socket.h"

namespace pulse::http {

Server::Server(Router router, const Server::Options& opts)
    : router_(std::move(router)), pool_(opts.threads), acceptor_(opts.port) {}

void Server::run() {
  while (true) {
    net::Socket s = acceptor_.accept();
    if (!s.ok()) {
      continue;
    }

    // TODO(use move-only functions)
    pool_.submit([this, socket = std::make_shared<net::Socket>(std::move(s))] {
      std::string header = socket->read_until("\r\n\r\n");
      if (header.empty()) {
        std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] empty header\n";
        return;
      }

      Result<Request> request = parse_header(header);
      if (!request.ok()) {
        std::cerr << "[" << __FILE__ << ":" << __LINE__
                  << "] parse error: " << pulse::to_string(request) << "\n";
        socket->write(serialize(Response{.content_type = "text/html",
                                         .status = 400,
                                         .body = "<h1>400 Bad Request</h1>"}));
        return;
      }

      std::optional<Router::Match> match =
          router_.match(request->method, request->path);
      if (!match.has_value()) {
        std::cerr << "[" << __FILE__ << ":" << __LINE__
                  << "] no routes for method: "
                  << pulse::to_string(request->method) << "\n";
        socket->write(serialize(Response{.content_type = "text/html",
                                         .status = 404,
                                         .body = "<h1>404 Not Found</h1>"}));
        return;
      }

      request->path_params = std::move(match->path_params);
      if (auto it = request->headers.find("Content-Length");
          it != request->headers.end()) {
        // TODO(write stoul to return success/failure)
        request->body = socket->read(std::stoul(it->second));
      }

      std::cerr << "[" << __FILE__ << ":" << __LINE__
                << "] request: " << pulse::to_string(request) << "\n";

      Response response = (*match->handler)(*request);
      socket->write(serialize(response));

      std::cerr << "[" << __FILE__ << ":" << __LINE__
                << "] response: " << pulse::to_string(response) << "\n";
    });
  }
}

}  // namespace pulse::http

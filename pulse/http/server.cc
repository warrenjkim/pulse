#include "pulse/http/server.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "pulse/concurrent/thread_pool.h"
#include "pulse/core/log.h"
#include "pulse/core/result.h"
#include "pulse/core/stringify.h"
#include "pulse/http/handler.h"
#include "pulse/http/internal/transform.h"
#include "pulse/http/request.h"
#include "pulse/http/response.h"
#include "pulse/http/router.h"
#include "pulse/net/acceptor.h"
#include "pulse/net/socket.h"

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
        Log() << "empty header";
        return;
      }

      Result<Request> request = parse_header(header);
      if (!request.ok()) {
        Log() << "parse error: " << request.error().message;
        socket->write(serialize(Response{.content_type = "text/html",
                                         .status = 400,
                                         .body = "<h1>400 Bad Request</h1>"}));
        return;
      }

      std::optional<Router::Match> match =
          router_.match(request->method, request->path);
      if (!match.has_value()) {
        Log() << "no routes for method: " << pulse::to_string(request->method);
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

      Log() << "request: " << pulse::to_string(*request);
      Response response = (*match->handler)(*request);
      socket->write(serialize(response));
      Log() << "response: " << pulse::to_string(response);
    });
  }
}

}  // namespace pulse::http

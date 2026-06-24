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
#include "pulse/http/internal/cors.h"
#include "pulse/http/internal/transform.h"
#include "pulse/http/method.h"
#include "pulse/http/request.h"
#include "pulse/http/response.h"
#include "pulse/http/router.h"
#include "pulse/net/acceptor.h"
#include "pulse/net/socket.h"

namespace pulse::http {

Server::Server(Router router, const Server::Options& opts)
    : router_(std::move(router)), pool_(opts.threads), acceptor_(opts.port) {}

void Server::Run() {
  while (true) {
    net::Socket s = acceptor_.Accept();
    if (!s.ok()) {
      continue;
    }
    // TODO(use move-only functions)
    pool_.Submit([this, socket = std::make_shared<net::Socket>(std::move(s))] {
      std::string header = socket->ReadUntil("\r\n\r\n");
      if (header.empty()) {
        Log() << "empty header";
        return;
      }

      Result<Request> request = ParseHeader(header);
      if (!request.ok()) {
        Log() << "parse error: " << request.error().message;
        socket->Write(Serialize(Response{.content_type = "text/html",
                                         .status = 400,
                                         .body = "<h1>400 Bad Request</h1>"}));
        return;
      }

      if (request->method == Method::kOptions) {
        socket->Write(Serialize(CorsPreflight(*request)));
        return;
      }

      std::optional<Router::RouteMatch> match =
          router_.Match(request->method, request->url);
      if (!match.has_value()) {
        Log() << "no routes for method: " << pulse::ToString(request->method);
        socket->Write(Serialize(Response{.content_type = "text/html",
                                         .status = 404,
                                         .body = "<h1>404 Not Found</h1>"}));
        return;
      }

      request->path = std::move(match->path_params);
      if (auto it = request->headers.find("Content-Length");
          it != request->headers.end()) {
        // TODO(write stoul to return success/failure)
        request->body = socket->Read(std::stoul(it->second));
      }

      Log() << "request: " << pulse::ToString(*request);
      Response response = (*match->handler)(*request);
      AddCorsHeaders(&response);
      socket->Write(Serialize(response));
      Log() << "response: " << pulse::ToString(response);
    });
  }
}

}  // namespace pulse::http

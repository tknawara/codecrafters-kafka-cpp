#pragma once

#include <asio.hpp>

#include "api/handler/request_handler.hpp"

using asio::ip::tcp;

namespace kafka {
class ClientSession {
public:
  ClientSession(tcp::socket client_socket, kafka::RequestHandler &handler)
      : client_socket_(std::move(client_socket)), request_handler_(handler) {}

  void handle_connection();

private:
  tcp::socket client_socket_;
  kafka::RequestHandler &request_handler_;
};
}; // namespace kafka
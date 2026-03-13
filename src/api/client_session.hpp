#pragma once

#include <asio.hpp>

#include "api/router.hpp"

using asio::ip::tcp;

namespace kafka {
class ClientSession {
public:
  ClientSession(tcp::socket client_socket, KafkaRouter &router)
      : client_socket_(std::move(client_socket)), router_(router) {}

  void handle_connection();

private:
  tcp::socket client_socket_;
  kafka::KafkaRouter &router_;
};
}; // namespace kafka
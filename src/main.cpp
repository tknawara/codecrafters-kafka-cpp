#include <arpa/inet.h>
#include <asio.hpp>
#include <cstdlib>
#include <cstring>
#include <fmt/format.h>
#include <iostream>
#include <netdb.h>
#include <print>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "client_session.hpp"
#include "request_handler.hpp"

using asio::ip::tcp;

int main() {
  // Disable output buffering
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  asio::io_context io_context;
  tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 9092));
  acceptor.set_option(tcp::acceptor::reuse_address(true));
  kafka::RequestHandler request_handler{};

  while (true) {
    try {
      std::println("Waiting for a client to connect...");

      tcp::socket socket(io_context);
      acceptor.accept(socket);

      std::cout << "Client connected from: " << socket.remote_endpoint()
                << "\n";

      std::thread client_thread([client_socket = std::move(socket),
                                 &request_handler]() mutable {
        kafka::ClientSession session(std::move(client_socket), request_handler);
        session.handle_connection();
      });

      client_thread.detach();

    } catch (std::exception &e) {
      std::cerr << "Exception: " << e.what() << "\n";
    }
  }

  return 0;
}
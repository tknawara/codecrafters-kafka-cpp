#include <arpa/inet.h>
#include <asio.hpp>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fmt/format.h>
#include <iostream>
#include <netdb.h>
#include <print>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "handler.hpp"
#include "request.hpp"
#include "response.hpp"

using asio::ip::tcp;

namespace {
void handle_connection(tcp::socket client_socket,
                       kafka::Handler &request_handler) {
  while (true) {
    uint32_t request_size_raw;
    asio::error_code error_code;
    asio::read(client_socket, asio::buffer(&request_size_raw, 4), error_code);

    if (error_code == asio::error::eof) {
      std::println("Client disconnected.");
      break;
    } else if (error_code) {
      std::cerr << "Read error: " << error_code.message() << '\n';
      break;
    }

    uint32_t request_size = std::byteswap(request_size_raw);
    std::vector<uint8_t> request_body(request_size);
    asio::read(client_socket, asio::buffer(request_body), error_code);

    if (error_code) {
      std::cerr << "Error reading request body: " << error_code.message()
                << '\n';
      break;
    }

    auto request =
        kafka::request::from_bytes(request_size, std::move(request_body));
    auto response = request_handler.handle(request);
    std::vector<uint8_t> response_buffer;
    kafka::response::serialize(response_buffer, response);
    asio::write(client_socket, asio::buffer(response_buffer), error_code);

    if (error_code) {
      std::cerr << "Error writing response: " << error_code.message() << "\n";
      break;
    }

    std::println("Sent response with Correlation ID");
  }
}
} // namespace

int main(int argc, char *argv[]) {
  // Disable output buffering
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  try {
    asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 9092));
    acceptor.set_option(tcp::acceptor::reuse_address(true));
    kafka::Handler handler{};

    std::println("Waiting for a client to connect...");

    tcp::socket socket(io_context);
    acceptor.accept(socket);

    std::cout << "Client connected from: " << socket.remote_endpoint() << "\n";

    handle_connection(std::move(socket), handler);
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}
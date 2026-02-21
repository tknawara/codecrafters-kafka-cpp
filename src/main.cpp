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

int main(int argc, char *argv[]) {
  // Disable output buffering
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  try {
    asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 9092));
    acceptor.set_option(tcp::acceptor::reuse_address(true));

    std::println("Waiting for a client to connect...");

    tcp::socket socket(io_context);
    acceptor.accept(socket);

    kafka::Handler handler{};

    std::cout << "Client connected from: " << socket.remote_endpoint() << "\n";

    uint32_t request_size_raw;
    asio::read(socket, asio::buffer(&request_size_raw, 4));

    uint32_t request_size = std::byteswap(request_size_raw);
    std::vector<uint8_t> request_body(request_size);
    asio::read(socket, asio::buffer(request_body));

    auto request =
        kafka::request::from_bytes(request_size, std::move(request_body));
    auto response = handler.handle(request);
    asio::write(socket, asio::buffer(kafka::response::serialize(response)));

    std::println("Sent response with Correlation ID");
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}
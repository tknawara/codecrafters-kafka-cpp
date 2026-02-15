#include <arpa/inet.h>
#include <asio.hpp>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fmt/format.h>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using asio::ip::tcp;

int main(int argc, char *argv[]) {
  // Disable output buffering
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  try {
    asio::io_context io_context;

    // Create a physical "acceptor" that listens on Port 9092
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 9092));

    // Set SO_REUSEADDR (Asio does this by default or via options)
    acceptor.set_option(tcp::acceptor::reuse_address(true));

    std::cout << "Waiting for a client to connect...\n";

    // This replaces the raw 'int client_fd' with a high-level socket object
    tcp::socket socket(io_context);
    acceptor.accept(socket);

    std::cout << "Client connected from: " << socket.remote_endpoint() << "\n";

    // 1. Read Request Size (4 bytes)
    uint32_t request_size_raw;
    asio::read(socket, asio::buffer(&request_size_raw, 4));

    // 2. Read the rest of the message based on request_size
    uint32_t request_size = std::byteswap(request_size_raw);
    std::vector<uint8_t> request_body(request_size);
    asio::read(socket, asio::buffer(request_body));

    // 3. Extract Correlation ID (The ID starts at offset 4 of the body)
    // Request Body structure: ApiKey(2) + ApiVersion(2) + CorrelationID(4)
    uint32_t correlation_id_raw;
    std::memcpy(&correlation_id_raw, &request_body[4], 4);

    // 4. Prepare Response
    // Response = Size(4) + CorrelationID(4)
    uint32_t response_correlation_id = correlation_id_raw; // Already in network order from client
    uint32_t response_body_size = std::byteswap((uint32_t)4);

    // 5. Send Response
    asio::write(socket, asio::buffer(&response_body_size, 4));
    asio::write(socket, asio::buffer(&response_correlation_id, 4));

    std::cout << "Sent response with Correlation ID\n";

    // Socket closes automatically when it leaves this scope!
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}
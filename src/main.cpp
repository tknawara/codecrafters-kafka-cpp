#include <arpa/inet.h>
#include <asio.hpp>
#include <cstdlib>
#include <cstring>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <print>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "api/handler/client_session.hpp"
#include "api/handler/request_handler.hpp"
#include "metadata/cache.hpp"
#include "metadata/parser.hpp"

using asio::ip::tcp;

int main() {
  // Disable output buffering
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  asio::io_context io_context;
  tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 9092));
  acceptor.set_option(tcp::acceptor::reuse_address(true));

  kafka::metadata::MetadataCache metadata_cache;

  std::string log_file_path =
      "/tmp/kraft-combined-logs/__cluster_metadata-0/00000000000000000000.log";

  kafka::metadata::MetadataParser::parse_log_file(log_file_path,
                                                  metadata_cache);
  kafka::RequestHandler request_handler{metadata_cache};

  std::ifstream log_file(
      "/tmp/kraft-combined-logs/__cluster_metadata-0/00000000000000000000.log",
      std::ios::binary);

  if (log_file) {
    std::cerr << "\n--- BEGIN SAFE HEX DUMP ---\n";
    unsigned char byte_val;
    while (log_file.read(reinterpret_cast<char *>(&byte_val), 1)) {
      // Print exactly 2 hex characters per byte with zero padding, no spaces
      std::cerr << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(byte_val);
    }
    std::cerr << "\n--- END SAFE HEX DUMP ---\n";

    // Reset standard output back to decimal just in case
    std::cerr << std::dec;
  }

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
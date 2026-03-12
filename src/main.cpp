#include <arpa/inet.h>
#include <asio.hpp>
#include <backward.hpp>
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

#include "api/handler/client_session.hpp"
#include "api/middleware.hpp"
#include "api/router.hpp"
#include "core/hexdump.hpp"
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

  std::string log_file_path =
      "/tmp/kraft-combined-logs/__cluster_metadata-0/00000000000000000000.log";

  auto cache = kafka::metadata::MetadataParser::parse_log_file(log_file_path);
  kafka::KafkaController controller{cache};

  kafka::KafkaRouter router =
      kafka::KafkaRouterBuilder{controller}
          .use(kafka::api::middleware::version_validator_middleware)
          .use(kafka::api::middleware::logging_middleware)
          .build();

  hexdump::dump_file(log_file_path);

  while (true) {
    try {
      std::println("Waiting for a client to connect...");

      tcp::socket socket(io_context);
      acceptor.accept(socket);

      std::cout << "Client connected from: " << socket.remote_endpoint()
                << "\n";

      std::thread client_thread(
          [client_socket = std::move(socket), &router]() mutable {
            kafka::ClientSession session(std::move(client_socket), router);
            session.handle_connection();
          });

      client_thread.detach();

    } catch (std::exception &e) {
      std::cerr << "Exception: " << e.what() << "\n";
      backward::StackTrace st;
      st.load_here(32);
      backward::Printer p;
      p.print(st, std::cerr);
    }
  }

  return 0;
}
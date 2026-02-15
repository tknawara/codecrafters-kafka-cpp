#include <arpa/inet.h>
#include <asio.hpp>
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

    // Socket closes automatically when it leaves this scope!
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;

  // int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  // if (server_fd < 0) {
  //   std::cerr << "Failed to create server socket: " << std::endl;
  //   return 1;
  // }

  // // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // // ensures that we don't run into 'Address already in use' errors
  // int reuse = 1;
  // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
  //   close(server_fd);
  //   std::cerr << "setsockopt failed: " << std::endl;
  //   return 1;
  // }

  // struct sockaddr_in server_addr{};
  // server_addr.sin_family = AF_INET;
  // server_addr.sin_addr.s_addr = INADDR_ANY;
  // server_addr.sin_port = htons(9092);

  // if (bind(server_fd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) != 0) {
  //   close(server_fd);
  //   std::cerr << "Failed to bind to port 9092" << std::endl;
  //   return 1;
  // }

  // int connection_backlog = 5;
  // if (listen(server_fd, connection_backlog) != 0) {
  //   close(server_fd);
  //   std::cerr << "listen failed" << std::endl;
  //   return 1;
  // }

  // std::cout << "Waiting for a client to connect...\n";

  // struct sockaddr_in client_addr{};
  // socklen_t client_addr_len = sizeof(client_addr);

  // // You can use print statements as follows for debugging, they'll be visible
  // // when running tests.
  // std::cerr << "Logs from your program will appear here!\n";

  // int client_fd = accept(server_fd, reinterpret_cast<struct sockaddr *>(&client_addr), &client_addr_len);
  // std::cout << "Client connected\n";
  // close(client_fd);

  // close(server_fd);
  // return 0;
}
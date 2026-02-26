#include "client_session.hpp"

#include "request.hpp"
#include "response.hpp"
#include <iostream>
#include <print>

void kafka::ClientSession::handle_connection() {
  while (true) {
    uint32_t request_size_raw;
    asio::error_code error_code;
    asio::read(client_socket_, asio::buffer(&request_size_raw, 4), error_code);

    if (error_code == asio::error::eof) {
      std::println("Client disconnected.");
      break;
    } else if (error_code) {
      std::cerr << "Read error: " << error_code.message() << '\n';
      break;
    }

    uint32_t request_size = std::byteswap(request_size_raw);
    std::vector<uint8_t> request_body(request_size);
    asio::read(client_socket_, asio::buffer(request_body), error_code);

    if (error_code) {
      std::cerr << "Error reading request body: " << error_code.message()
                << '\n';
      break;
    }

    auto request =
        kafka::request::from_bytes(request_size, std::move(request_body));
    auto response = request_handler_.handle(request);
    std::vector<uint8_t> response_buffer;
    kafka::response::serialize(response_buffer, response);
    asio::write(client_socket_, asio::buffer(response_buffer), error_code);

    if (error_code) {
      std::cerr << "Error writing response: " << error_code.message() << "\n";
      break;
    }

    std::println("Sent response with Correlation ID");
  }
}
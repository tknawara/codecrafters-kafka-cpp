#pragma once

#include <inttypes.h>
#include <vector>

#include "api_details.hpp"

namespace kafka::request {

struct Header {
  api::metadata::ApiKey api;
  uint16_t version;
  uint32_t correlation_id;
};

struct Request {
  uint32_t message_size;
  Header header;
  std::vector<uint8_t> body;
};

Request from_bytes(uint32_t message_size, std::vector<uint8_t> raw);

}; // namespace kafka::request
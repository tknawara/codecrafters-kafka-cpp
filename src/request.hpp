#pragma once

#include <inttypes.h>
#include <vector>

namespace kafka::request {

enum class KafkaApi : uint16_t { ApiVersions = 18 };

struct Header {
  KafkaApi api;
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
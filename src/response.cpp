#include <bit>
#include <cstring>

#include "response.hpp"
#include "writer.hpp"

void kafka::response::serialize(std::vector<uint8_t> &buffer,
                                const ApiVersionKey &key) {
  writer::write_be(buffer, key.api_key);
  writer::write_be(buffer, key.min_version);
  writer::write_be(buffer, key.max_version);
}

void kafka::response::serialize(std::vector<uint8_t> &buffer,
                                const ApiVersionsResponse &body) {
  writer::write_be(buffer, static_cast<uint16_t>(body.error));
  writer::write_compact_array(buffer, body.keys);
  writer::write_be(buffer, body.throttle_time_ms);

  // Tagged fields for the body
  buffer.push_back(0);
}

struct ResponseVisitor {
  std::vector<uint8_t> &buffer;

  void operator()(const kafka::response::ApiVersionsResponse &body) const {
    serialize(buffer, body);
  }
};

void kafka::response::serialize(std::vector<uint8_t> &buffer,
                                const Response &res) {
  // message size
  for (int i = 0; i < 4; ++i) {
    buffer.push_back(0);
  }
  writer::write_be(buffer, res.correlation_id);
  std::visit(ResponseVisitor{buffer}, res.body);

  uint32_t total_size = std::byteswap(static_cast<uint32_t>(buffer.size() - 4));
  std::memcpy(buffer.data(), &total_size, 4);
}
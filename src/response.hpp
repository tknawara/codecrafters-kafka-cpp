#pragma once

#include <inttypes.h>
#include <variant>
#include <vector>

#include "error.hpp"

namespace kafka::response {

struct ApiVersionKey {
  uint16_t api_key;
  uint16_t min_version;
  uint16_t max_version;
};

struct ApiVersionsResponse {
  error::ErrorCode error;
  std::vector<ApiVersionKey> keys;
  uint32_t throttle_time_ms;
};

using ResponseBody = std::variant<ApiVersionsResponse>;

struct Response {
  uint32_t correlation_id;
  ResponseBody body;
};

void serialize(std::vector<uint8_t> &buffer, const ApiVersionKey &key);
void serialize(std::vector<uint8_t> &buffer,
               const ApiVersionsResponse &response);
void serialize(std::vector<uint8_t> &buffer, const Response &response);

}; // namespace kafka::response
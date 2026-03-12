#pragma once

#include <cstdint>

namespace kafka::api::registry {
enum class ApiKey : uint16_t {
  Produce = 0,
  Fetch = 1,
  ApiVersions = 18,
  DescribeTopicParititons = 75,
};

int16_t get_request_header_version(ApiKey api, int16_t version);
} // namespace kafka::api::registry
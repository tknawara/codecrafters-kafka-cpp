#pragma once

#include <cstdint>

namespace kafka::api::registry {
enum class ApiKey : uint16_t {
  ApiVersions = 18,
  DescribeTopicParititons = 75,
  Fetch = 1
};

int16_t get_request_header_version(ApiKey api, int16_t version);
} // namespace kafka::api::registry
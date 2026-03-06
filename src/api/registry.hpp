#pragma once

#include <cinttypes>

namespace kafka::api::registry {
enum class ApiKey : uint16_t { ApiVersions = 18, DescribeTopicParititons = 75 };
}
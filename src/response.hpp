#pragma once

#include <inttypes.h>
#include <vector>

#include "error.hpp"

namespace kafka::response {

struct Response {
  uint32_t correlation_id;
  error::ErrorCode error;
};

std::vector<uint8_t> serialize(const Response &response);

}; // namespace kafka::response
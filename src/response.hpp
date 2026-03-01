#pragma once

#include <inttypes.h>
#include <variant>

#include "api_details.hpp"

namespace kafka::response {

using ResponseBody = std::variant<api::dto::ApiVersionsResponse>;

struct Response {
  uint32_t correlation_id;
  ResponseBody body;
};

}; // namespace kafka::response

namespace kafka {} // namespace kafka
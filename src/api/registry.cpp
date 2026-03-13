#include "api/registry.hpp"

int16_t kafka::api::registry::get_request_header_version(ApiKey api,
                                                         int16_t version) {
  switch (api) {
  case ApiKey::ApiVersions:
    return 1; // APIVersions v0-v3 always uses Request Header v1
  case ApiKey::DescribeTopicParititons:
    return 2; // Uses flexible Header v2
  case ApiKey::Fetch:
    return (version >= 12) ? 2 : 1; // Fetch v16 uses flexible Header v2
  case ApiKey::Produce:
    return 2;
  default:
    return 1; // Default fallback
  }
}
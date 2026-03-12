#pragma once

#include <array>
#include <optional>
#include <utility>
#include <vector>

#include "api/registry.hpp"
#include "core/writer.hpp"

namespace kafka::api::dto {

struct ApiDetails {
  registry::ApiKey key;
  int16_t min_version;
  int16_t max_version;
};

struct ApiVersionsResponse {
  static constexpr uint8_t header_version = 0;

  int16_t error_code{0};
  std::vector<ApiDetails> keys;
  uint32_t throttle_time_ms;
};

inline constexpr auto supported_apis = std::to_array<ApiDetails>({
    {registry::ApiKey::Fetch, 0, 16},
    {registry::ApiKey::Produce, 0, 11},
    {registry::ApiKey::ApiVersions, 0, 4},
    {registry::ApiKey::DescribeTopicParititons, 0, 4},
});

auto get_api_details(uint16_t raw_key) -> std::optional<ApiDetails>;
auto get_all_api_details() -> std::vector<ApiDetails>;

bool is_supported_version(registry::ApiKey key, uint16_t version);

} // namespace kafka::api::dto

namespace kafka {

template <> struct Serializer<api::dto::ApiDetails> {
  static void serialize(std::vector<uint8_t> &buffer,
                        const api::dto::ApiDetails &api_details) {

    writer::write_be(buffer, std::to_underlying(api_details.key));
    writer::write_be(buffer, api_details.min_version);
    writer::write_be(buffer, api_details.max_version);

    buffer.push_back(0); // TAG_BUFFER
  }
};

template <> struct Serializer<api::dto::ApiVersionsResponse> {

  static void serialize(std::vector<uint8_t> &buffer,
                        const api::dto::ApiVersionsResponse &body) {
    writer::write_be(buffer, body.error_code);
    writer::write_compact_array(buffer, body.keys);
    writer::write_be(buffer, body.throttle_time_ms);

    // Tagged fields for the body
    buffer.push_back(0);
  }
};
} // namespace kafka
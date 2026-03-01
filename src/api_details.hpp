#pragma once

#include <array>
#include <inttypes.h>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "error.hpp"
#include "serializable.hpp"
#include "writer.hpp"

namespace kafka::api {
namespace metadata {
enum class ApiKey : uint16_t { ApiVersions = 18, DescribeTopicParititons = 75 };

struct ApiDetails {
  ApiKey key;
  int16_t min_version;
  int16_t max_version;
};

inline constexpr auto supported_apis = std::to_array<ApiDetails>({
    {ApiKey::ApiVersions, 0, 4},
    {ApiKey::DescribeTopicParititons, 0, 4},
});

auto get_api_details(uint16_t raw_key) -> std::optional<ApiDetails>;
auto get_all_api_details() -> std::vector<ApiDetails>;
} // namespace metadata

namespace dto {
struct ApiVersionsResponse {
  error::ErrorCode error;
  std::vector<api::metadata::ApiDetails> keys;
  uint32_t throttle_time_ms;
};

using ResponseBody = std::variant<api::dto::ApiVersionsResponse>;

struct Response {
  uint32_t correlation_id;
  ResponseBody body;
};

} // namespace dto

} // namespace kafka::api

namespace kafka {

template <> struct Serializer<api::metadata::ApiDetails> {
  static void serialize(std::vector<uint8_t> &buffer,
                        const api::metadata::ApiDetails &api_details) {

    writer::write_be(buffer, std::to_underlying(api_details.key));
    writer::write_be(buffer, api_details.min_version);
    writer::write_be(buffer, api_details.max_version);
  }
};

template <> struct Serializer<api::dto::ApiVersionsResponse> {

  static void serialize(std::vector<uint8_t> &buffer,
                        const api::dto::ApiVersionsResponse &body) {
    writer::write_be(buffer, static_cast<uint16_t>(body.error));
    writer::write_compact_array(buffer, body.keys);
    writer::write_be(buffer, body.throttle_time_ms);

    // Tagged fields for the body
    buffer.push_back(0);
  }
};

template <> struct Serializer<api::dto::Response> {
  static void serialize(std::vector<uint8_t> &buffer,
                        const api::dto::Response &res) {
    // message size
    size_t size_offset = buffer.size();
    for (int i = 0; i < 4; ++i) {
      buffer.push_back(0);
    }
    writer::write_be(buffer, res.correlation_id);
    std::visit(
        [&buffer](const auto &body) {
          using BodyType = std::decay_t<decltype(body)>;
          Serializer<BodyType>::serialize(buffer, body);
        },
        res.body);

    uint32_t message_size =
        static_cast<uint32_t>(buffer.size() - size_offset - 4);
    uint32_t swapped_size = std::byteswap(message_size);

    std::memcpy(buffer.data() + size_offset, &swapped_size, 4);
  }
};
} // namespace kafka
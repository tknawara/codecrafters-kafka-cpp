#pragma once

#include "describe_topic_partitions.hpp"

#include <inttypes.h>
#include <variant>
#include <vector>

#include "api_versions.hpp"
#include "core/error.hpp"
#include "core/serializable.hpp"
#include "core/writer.hpp"

namespace kafka::api::dto {

using ResponseBody =
    std::variant<ApiVersionsResponse, DescribeTopicPartitionsResponse>;

struct Response {
  uint32_t correlation_id;
  ResponseBody body;
};

} // namespace kafka::api::dto

namespace kafka {
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
          if constexpr (BodyType::header_version == 1) {
            buffer.push_back(0);
          }
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
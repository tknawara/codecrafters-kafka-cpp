#pragma once

#include <array>
#include <string>
#include <vector>

#include "core/deserializable.hpp"
#include "core/primitive_serializers.hpp"
#include "core/reader.hpp"
#include "core/serializable.hpp"
#include "core/writer.hpp"

namespace kafka::api::dto {
struct DescribeTopicPartitionsRequestTopic {
  std::string topic_name;
};

struct DescribeTopicPartitionsRequest {
  std::vector<DescribeTopicPartitionsRequestTopic> topics;
};

struct DescribeTopicPartitionsResponseTopic {
  int16_t error_code{0};
  std::string name;
  std::array<uint8_t, 16> topic_id;
  bool is_internal{false};
  std::vector<int32_t> partitions{};
  int32_t topic_authorized_operations{0x00000000};
};

struct DescribeTopicPartitionsResponse {
  int32_t throttle_time_ms{0};
  std::vector<DescribeTopicPartitionsResponseTopic> topics;
  int8_t next_cursor{-1};
};

}; // namespace kafka::api::dto

namespace kafka {

template <> struct Serializer<api::dto::DescribeTopicPartitionsResponseTopic> {
  static void
  serialize(std::vector<uint8_t> &buffer,
            const api::dto::DescribeTopicPartitionsResponseTopic &topic) {

    writer::write_be(buffer, topic.error_code);
    writer::write_compact_string(buffer, topic.name);

    // Write the 16-byte UUID
    for (uint8_t byte : topic.topic_id) {
      buffer.push_back(byte);
    }

    writer::write_be(buffer, topic.is_internal);
    writer::write_compact_array(buffer, topic.partitions);
    writer::write_be(buffer, topic.topic_authorized_operations);

    buffer.push_back(0); // TAG_BUFFER for the topic
  }
};

template <> struct Serializer<api::dto::DescribeTopicPartitionsResponse> {
  static void serialize(std::vector<uint8_t> &buffer,
                        const api::dto::DescribeTopicPartitionsResponse &res) {

    writer::write_be(buffer, res.throttle_time_ms);
    writer::write_compact_array(buffer, res.topics);
    writer::write_be(buffer, res.next_cursor);

    buffer.push_back(0); // TAG_BUFFER for the overall body
  }
};

} // namespace kafka

namespace kafka {

template <> struct Deserializer<api::dto::DescribeTopicPartitionsRequestTopic> {
  static api::dto::DescribeTopicPartitionsRequestTopic
  deserialize(std::span<const uint8_t> buffer, size_t &offset) {

    api::dto::DescribeTopicPartitionsRequestTopic topic;

    // You will need a reader::read_compact_string utility
    topic.topic_name = reader::read_compact_string(buffer, offset);

    // Skip the TAG_BUFFER (1 byte if empty)
    offset += 1;

    return topic;
  }
};

template <> struct Deserializer<api::dto::DescribeTopicPartitionsRequest> {
  static api::dto::DescribeTopicPartitionsRequest
  deserialize(std::span<const uint8_t> buffer, size_t &offset) {

    api::dto::DescribeTopicPartitionsRequest req;

    req.topics = reader::read_compact_array<
        api::dto::DescribeTopicPartitionsRequestTopic>(buffer, offset);

    // Skip the TAG_BUFFER (1 byte if empty)
    offset += 1;

    return req;
  }
};

} // namespace kafka
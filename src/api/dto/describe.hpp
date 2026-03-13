#pragma once

#include <array>
#include <string>
#include <vector>

#include "core/deserializable.hpp"
#include "core/primitive_serializers.hpp" // IWYU pragma: keep
#include "core/reader.hpp"
#include "core/serializable.hpp"
#include "core/writer.hpp"

namespace kafka::api::dto {
struct DescribeTopicRequest {
  std::string name;
};

struct DescribeRequest {
  std::vector<DescribeTopicRequest> topics;
};

struct DescribeTopicPartitionResponse {
  int16_t error_code{0};
  int32_t partition_index{0};
  int32_t leader_id{0};
  int32_t leader_epoch{0};
  std::vector<int32_t> replica_nodes;
  std::vector<int32_t> isr_nodes;
  std::vector<int32_t> eligible_leader_replicas;
  std::vector<int32_t> last_known_elr;
  std::vector<int32_t> offline_replicas;
};

struct DescribeTopicResponse {
  int16_t error_code{0};
  std::string name;
  std::array<uint8_t, 16> topic_id;
  bool is_internal{false};
  std::vector<DescribeTopicPartitionResponse> partitions{};
  int32_t topic_authorized_operations{0x00000000};
};

struct DescribeResponse {
  static constexpr uint8_t header_version = 1;

  int32_t throttle_time_ms{0};
  std::vector<DescribeTopicResponse> topics;
  int8_t next_cursor{-1};
};

}; // namespace kafka::api::dto

namespace kafka {

template <> struct Serializer<api::dto::DescribeTopicPartitionResponse> {
  static void
  serialize(std::vector<uint8_t> &buffer,
            const api::dto::DescribeTopicPartitionResponse &partition) {

    writer::write_be(buffer, partition.error_code);
    writer::write_be(buffer, partition.partition_index);
    writer::write_be(buffer, partition.leader_id);
    writer::write_be(buffer, partition.leader_epoch);

    writer::write_compact_array(buffer, partition.replica_nodes);
    writer::write_compact_array(buffer, partition.isr_nodes);
    writer::write_compact_array(buffer, partition.eligible_leader_replicas);
    writer::write_compact_array(buffer, partition.last_known_elr);
    writer::write_compact_array(buffer, partition.offline_replicas);

    // The trailing TAG_BUFFER for the partition struct
    buffer.push_back(0);
  }
};

template <> struct Serializer<api::dto::DescribeTopicResponse> {
  static void serialize(std::vector<uint8_t> &buffer,
                        const api::dto::DescribeTopicResponse &topic) {

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

template <> struct Serializer<api::dto::DescribeResponse> {
  static void serialize(std::vector<uint8_t> &buffer,
                        const api::dto::DescribeResponse &res) {

    writer::write_be(buffer, res.throttle_time_ms);
    writer::write_compact_array(buffer, res.topics);
    writer::write_be(buffer, res.next_cursor);

    buffer.push_back(0); // TAG_BUFFER for the overall body
  }
};

} // namespace kafka

namespace kafka {

template <> struct Deserializer<api::dto::DescribeTopicRequest> {
  static api::dto::DescribeTopicRequest
  deserialize(std::span<const uint8_t> buffer, size_t &offset) {

    api::dto::DescribeTopicRequest topic;

    // You will need a reader::read_compact_string utility
    topic.name = reader::read_compact_string(buffer, offset);

    // Skip the TAG_BUFFER (1 byte if empty)
    offset += 1;

    return topic;
  }
};

template <> struct Deserializer<api::dto::DescribeRequest> {
  static api::dto::DescribeRequest deserialize(std::span<const uint8_t> buffer,
                                               size_t &offset) {

    api::dto::DescribeRequest req;

    req.topics = reader::read_compact_array<api::dto::DescribeTopicRequest>(
        buffer, offset);

    // Skip the TAG_BUFFER (1 byte if empty)
    offset += 1;

    return req;
  }
};

} // namespace kafka
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "core/deserializable.hpp"
#include "core/reader.hpp"
#include "core/serializable.hpp"
#include "core/writer.hpp"

namespace kafka::api::dto {

struct ProducePartitionRequest {
  int32_t index;
  std::optional<std::vector<uint8_t>> records;
};

struct ProduceTopicRequest {
  std::string name;
  std::vector<ProducePartitionRequest> partitions;
};

struct ProduceRequest {
  std::optional<std::string> transactional_id;
  int16_t acks;
  int32_t timeout_ms;
  std::vector<ProduceTopicRequest> topics;
};

struct ProduceRecordErrorResponse {
  int32_t batch_index;
  std::optional<std::string> batch_index_error_message;
};

struct ProducePartitionResponse {
  int32_t index;
  int16_t error_code;
  int64_t base_offset;
  int64_t log_append_time_ms;
  int64_t log_start_offset;
  std::vector<ProduceRecordErrorResponse> record_errors;
  std::optional<std::string> error_message;
};

struct ProduceTopicResponse {
  std::string name;
  std::vector<ProducePartitionResponse> partitions;
};

struct ProduceResponse {
  static constexpr uint8_t header_version = 1;

  std::vector<ProduceTopicResponse> responses;
  int32_t throttle_time_ms;
};

} // namespace kafka::api::dto

namespace kafka {

// 1. Partition Deserializer
template <> struct Deserializer<api::dto::ProducePartitionRequest> {
  static api::dto::ProducePartitionRequest
  deserialize(std::span<const uint8_t> buffer, size_t &offset) {
    api::dto::ProducePartitionRequest partition{};

    partition.index = reader::read_be<int32_t>(buffer, offset);
    partition.records = reader::read_compact_bytes(buffer, offset);

    reader::skip_tag_buffer(buffer, offset);
    return partition;
  }
};

// 2. Topic Deserializer
template <> struct Deserializer<api::dto::ProduceTopicRequest> {
  static api::dto::ProduceTopicRequest
  deserialize(std::span<const uint8_t> buffer, size_t &offset) {
    api::dto::ProduceTopicRequest topic{};

    topic.name = reader::read_compact_string(buffer, offset);
    topic.partitions =
        reader::read_compact_array<api::dto::ProducePartitionRequest>(buffer,
                                                                      offset);

    reader::skip_tag_buffer(buffer, offset);
    return topic;
  }
};

// 3. The Main Request Deserializer
template <> struct Deserializer<api::dto::ProduceRequest> {
  static api::dto::ProduceRequest deserialize(std::span<const uint8_t> buffer,
                                              size_t &offset) {
    api::dto::ProduceRequest req{};

    req.transactional_id = reader::read_nullable_compact_string(buffer, offset);
    req.acks = reader::read_be<int16_t>(buffer, offset);
    req.timeout_ms = reader::read_be<int32_t>(buffer, offset);
    req.topics = reader::read_compact_array<api::dto::ProduceTopicRequest>(
        buffer, offset);

    reader::skip_tag_buffer(buffer, offset);
    return req;
  }
};

} // namespace kafka

namespace kafka {

// 1. Record Error Serializer (for batch-level errors)
template <> struct Serializer<api::dto::ProduceRecordErrorResponse> {
  static void serialize(std::vector<uint8_t> &buffer,
                        const api::dto::ProduceRecordErrorResponse &error) {
    writer::write_be<int32_t>(buffer, error.batch_index);
    writer::write_nullable_compact_string(buffer,
                                          error.batch_index_error_message);
    writer::write_empty_tag_buffer(buffer);
  }
};

// 2. Partition Serializer
template <> struct Serializer<api::dto::ProducePartitionResponse> {
  static void serialize(std::vector<uint8_t> &buffer,
                        const api::dto::ProducePartitionResponse &partition) {
    writer::write_be<int32_t>(buffer, partition.index);
    writer::write_be<int16_t>(buffer, partition.error_code);
    writer::write_be<int64_t>(buffer, partition.base_offset);
    writer::write_be<int64_t>(buffer, partition.log_append_time_ms);
    writer::write_be<int64_t>(buffer, partition.log_start_offset);
    writer::write_compact_array(buffer, partition.record_errors);
    writer::write_nullable_compact_string(buffer, partition.error_message);
    writer::write_empty_tag_buffer(buffer);
  }
};

// 3. Topic Serializer
template <> struct Serializer<api::dto::ProduceTopicResponse> {
  static void serialize(std::vector<uint8_t> &buffer,
                        const api::dto::ProduceTopicResponse &topic) {
    writer::write_compact_string(buffer, topic.name);
    writer::write_compact_array(buffer, topic.partitions);
    writer::write_empty_tag_buffer(buffer);
  }
};

// 4. The Main Response Serializer
template <> struct Serializer<api::dto::ProduceResponse> {
  static void serialize(std::vector<uint8_t> &buffer,
                        const api::dto::ProduceResponse &response) {
    writer::write_compact_array(buffer, response.responses);
    writer::write_be<int32_t>(buffer, response.throttle_time_ms);
    writer::write_empty_tag_buffer(buffer);
  }
};

} // namespace kafka
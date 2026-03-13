#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

#include "core/deserializable.hpp"
#include "core/reader.hpp"
#include "core/serializable.hpp"
#include "core/writer.hpp"

namespace kafka::api::dto {

struct FetchPartitionRequest {
  int32_t index;
  int32_t current_leader_epoch;
  int64_t fetch_offset;
  int32_t last_fetched_epoch;
  int64_t log_start_offset;
  int32_t partition_max_bytes;
};

struct FetchTopicRequest {
  std::array<uint8_t, 16> topic_id;
  std::vector<FetchPartitionRequest> partitions;
};

struct FetchRequest {
  int32_t max_wait_ms;
  int32_t min_bytes;
  int32_t max_bytes;
  int8_t isolation_level;
  int32_t session_id;
  int32_t session_epoch;
  std::vector<FetchTopicRequest> topics;
};

struct FetchPartitionAbortedTransactionResponse {
  int64_t producer_id;
  int64_t first_offset;
};

struct FetchPartitionResponse {
  int32_t partition_index;
  int16_t error_code;
  int64_t high_watermark;
  int64_t last_stable_offset;
  int64_t log_start_offset;
  std::vector<FetchPartitionAbortedTransactionResponse> aborted_transactions;
  int32_t preferred_read_replica = -1;
  std::vector<uint8_t> records;
};

struct FetchTopicResponse {
  std::array<uint8_t, 16> topic_id;
  std::vector<FetchPartitionResponse> partitions;
};

struct FetchResponse {
  static constexpr uint8_t header_version = 1;

  int32_t throttle_time_ms = 0;
  int16_t error_code = 0;
  int32_t session_id = 0;
  std::vector<FetchTopicResponse> responses;
};

} // namespace kafka::api::dto

namespace kafka {

template <>
struct Serializer<api::dto::FetchPartitionAbortedTransactionResponse> {
  static void
  serialize(std::vector<uint8_t> &buffer,
            const api::dto::FetchPartitionAbortedTransactionResponse &txn) {
    writer::write_be<int64_t>(buffer, txn.producer_id);
    writer::write_be<int64_t>(buffer, txn.first_offset);
    writer::write_be<uint8_t>(buffer, 0x00); // TAG_BUFFER
  }
};

template <> struct Serializer<api::dto::FetchPartitionResponse> {
  static void serialize(std::vector<uint8_t> &buffer,
                        const api::dto::FetchPartitionResponse &response) {
    writer::write_be<int32_t>(buffer, response.partition_index);
    writer::write_be<int16_t>(buffer, response.error_code);
    writer::write_be<int64_t>(buffer, response.high_watermark);
    writer::write_be<int64_t>(buffer, response.last_stable_offset);
    writer::write_be<int64_t>(buffer, response.log_start_offset);
    writer::write_compact_array(buffer, response.aborted_transactions);
    // Preferred read replica
    writer::write_be<int32_t>(buffer, response.preferred_read_replica);
    // Records (compact nullable bytes)
    if (response.records.empty()) {
      writer::write_unsigned_varint(buffer, 0); // null
    } else {
      writer::write_unsigned_varint(buffer, response.records.size() + 1);
      buffer.insert(buffer.end(), response.records.begin(),
                    response.records.end());
    }

    // TAG_BUFFER
    writer::write_be<uint8_t>(buffer, 0x00);
  }
};

static_assert(Serializable<api::dto::FetchPartitionResponse>,
              "FetchPartitionResponse failed the Serializer contract!");

template <> struct Serializer<api::dto::FetchTopicResponse> {
  static void serialize(std::vector<uint8_t> &buffer,
                        const api::dto::FetchTopicResponse &response) {
    // Raw copy of the 16-byte UUID
    buffer.insert(buffer.end(), response.topic_id.begin(),
                  response.topic_id.end());

    writer::write_compact_array(buffer, response.partitions);

    // TAG_BUFFER
    writer::write_be<uint8_t>(buffer, 0x00);
  }
};

static_assert(Serializable<api::dto::FetchTopicResponse>,
              "FetchTopicResponse failed the Serializer contract!");

template <> struct Serializer<api::dto::FetchResponse> {
  static void serialize(std::vector<uint8_t> &buffer,
                        const api::dto::FetchResponse &response) {

    // 1. Top level fields
    writer::write_be<int32_t>(buffer, response.throttle_time_ms);
    writer::write_be<int16_t>(buffer, response.error_code);
    writer::write_be<int32_t>(buffer, response.session_id);

    // 2. Compact Array of topics (responses)
    // For Stage 8, response.responses is empty, so this writes a varint '1'
    writer::write_compact_array(buffer, response.responses);

    // 3. Top-level TAG_BUFFER for FetchResponse (empty = 0x00)
    writer::write_be<uint8_t>(buffer, 0x00);
  }
};

static_assert(Serializable<api::dto::FetchResponse>,
              "FetchResponse failed the Serializer contract!");

} // namespace kafka

namespace kafka {
template <> struct Deserializer<api::dto::FetchRequest> {
  static api::dto::FetchRequest deserialize(std::span<const uint8_t> buffer,
                                            size_t &offset) {
    api::dto::FetchRequest req{};

    // 1. Read top-level metadata
    req.max_wait_ms = reader::read_be<int32_t>(buffer, offset);
    req.min_bytes = reader::read_be<int32_t>(buffer, offset);
    req.max_bytes = reader::read_be<int32_t>(buffer, offset);
    req.isolation_level = reader::read_be<int8_t>(buffer, offset);
    req.session_id = reader::read_be<int32_t>(buffer, offset);
    req.session_epoch = reader::read_be<int32_t>(buffer, offset);

    // 2. Read Topics Compact Array Length (N + 1)
    uint32_t topics_length = reader::read_unsigned_varint(buffer, offset);
    uint32_t topics_count = (topics_length == 0) ? 0 : topics_length - 1;

    for (uint32_t i = 0; i < topics_count; ++i) {
      api::dto::FetchTopicRequest topic{};

      // Read the 16-byte UUID
      std::copy_n(buffer.begin() + offset, 16, topic.topic_id.begin());
      offset += 16;

      // Read Partitions Compact Array Length (N + 1)
      uint32_t partitions_length = reader::read_unsigned_varint(buffer, offset);
      uint32_t partitions_count =
          (partitions_length == 0) ? 0 : partitions_length - 1;

      for (uint32_t j = 0; j < partitions_count; ++j) {
        api::dto::FetchPartitionRequest partition{};

        partition.index = reader::read_be<int32_t>(buffer, offset);
        partition.current_leader_epoch =
            reader::read_be<int32_t>(buffer, offset);
        partition.fetch_offset = reader::read_be<int64_t>(buffer, offset);
        partition.last_fetched_epoch = reader::read_be<int32_t>(buffer, offset);
        partition.log_start_offset = reader::read_be<int64_t>(buffer, offset);
        partition.partition_max_bytes =
            reader::read_be<int32_t>(buffer, offset);

        // Read Partition TAG_BUFFER length and skip it
        uint32_t p_tags_length = reader::read_unsigned_varint(buffer, offset);
        // (If your varint parser doesn't advance offset for the contents of the
        // tag buffer, you might need a loop here to skip tag bytes, but usually
        // it's just 0x00 for empty tags)

        topic.partitions.push_back(partition);
      }

      // Read Topic TAG_BUFFER length and skip it
      uint32_t t_tags_length = reader::read_unsigned_varint(buffer, offset);

      req.topics.push_back(topic);
    }

    // Note: We can safely stop deserializing right here!
    // The request contains 'forgotten_topics_data' and 'rack_id' after this,
    // but since we only need the topic_id to build the response, we can just
    // return what we have.

    return req;
  }
};
} // namespace kafka
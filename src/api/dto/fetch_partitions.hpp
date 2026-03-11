#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "core/serializable.hpp"
#include "core/writer.hpp"

namespace kafka::api::dto {

struct FetchPartitionRequest {
  int32_t partition_id;
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

struct FetchPartitionResponse {
  int32_t partition_index;
  int16_t error_code;
  int64_t high_watermark;
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

template <> struct Serializer<api::dto::FetchPartitionResponse> {
  static void serialize(std::vector<uint8_t> &buffer,
                        const api::dto::FetchPartitionResponse &response) {
    writer::write_be<int32_t>(buffer, response.partition_index);
    writer::write_be<int16_t>(buffer, response.error_code);
    writer::write_be<int64_t>(buffer, response.high_watermark);

    // Note: We will add the raw record_batches array here in later stages!

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
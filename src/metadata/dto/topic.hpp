#pragma once

#include <algorithm>
#include <array>
#include <cinttypes>
#include <string>
#include <vector>

#include "core/deserializable.hpp"
#include "core/primitive_deserializers.hpp"
#include "core/reader.hpp"

namespace kafka::metadata::dto {
struct TopicRecord {
  std::string name;
  std::array<uint8_t, 16> topic_id;
};

struct PartitionRecord {
  int32_t partition_id;
  std::array<uint8_t, 16> topic_id{};
  std::vector<int32_t> replicas;
  std::vector<int32_t> isr;
  std::vector<int32_t> removing_replicas;
  std::vector<int32_t> adding_replicas;
  int32_t leader;
  int32_t leader_epoch;
  int32_t partition_epoch;
};

} // namespace kafka::metadata::dto

namespace kafka {
template <> struct Deserializer<metadata::dto::TopicRecord> {
  static metadata::dto::TopicRecord deserialize(std::span<const uint8_t> buffer,
                                                size_t &offset) {
    metadata::dto::TopicRecord record;
    record.name = reader::read_compact_string(buffer, offset);

    for (int i = 0; i < 16; ++i) {
      record.topic_id[i] = buffer[offset++];
    }

    // We stop right here! No TAG_BUFFER reading.
    // The framing layer will fast-forward the offset for us.
    return record;
  }
};

template <> struct Deserializer<metadata::dto::PartitionRecord> {
  static metadata::dto::PartitionRecord
  deserialize(std::span<const uint8_t> buffer, size_t &offset) {
    metadata::dto::PartitionRecord record;

    // 1. Read the Partition ID
    record.partition_id = reader::read_be<int32_t>(buffer, offset);

    // 2. Safely copy the 16-byte UUID
    std::copy_n(buffer.begin() + offset, 16, record.topic_id.begin());
    offset += 16;

    // 3. Read the four compact arrays of broker IDs
    record.replicas = reader::read_compact_array<int32_t>(buffer, offset);
    record.isr = reader::read_compact_array<int32_t>(buffer, offset);
    record.removing_replicas =
        reader::read_compact_array<int32_t>(buffer, offset);
    record.adding_replicas =
        reader::read_compact_array<int32_t>(buffer, offset);

    // 4. Read the leader and epoch metadata
    record.leader = reader::read_be<int32_t>(buffer, offset);
    record.leader_epoch = reader::read_be<int32_t>(buffer, offset);
    record.partition_epoch = reader::read_be<int32_t>(buffer, offset);

    // We stop right here! No TAG_BUFFER reading.
    // The framing layer will fast-forward the offset for us.
    return record;
  }
};

} // namespace kafka
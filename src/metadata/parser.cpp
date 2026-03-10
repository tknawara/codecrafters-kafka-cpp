#include "metadata/parser.hpp"

#include <fstream>
#include <print>
#include <span>
#include <vector>

#include "core/reader.hpp"

auto kafka::metadata::MetadataParser::parse_log_file(
    const std::string &filepath) -> MetadataCache {
  MetadataCache cache;
  // 1. Read the entire file into memory
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (!file) {
    return cache;
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  std::vector<uint8_t> buffer(size);
  if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
    return cache;
  }

  size_t offset = 0;

  // 2. Traverse the RecordBatches
  while (offset < buffer.size()) {
    if (offset + 12 > buffer.size())
      break; // Safety check

    size_t batch_start = offset;

    // Skip baseOffset (8 bytes)
    offset += 8;

    int32_t batch_length = reader::read_be<int32_t>(buffer, offset);
    size_t batch_end = batch_start + 12 + batch_length;

    // Fast-forward 45 bytes directly to the records_count field
    // (Skips CRC, timestamps, producer IDs, etc.)
    offset = batch_start + 57;
    int32_t records_count = reader::read_be<int32_t>(buffer, offset);

    // 3. Traverse the Records inside the batch
    for (int i = 0; i < records_count; ++i) {
      int32_t record_length = reader::read_signed_varint(buffer, offset);
      size_t record_end = offset + record_length;

      // Skip attributes (1 byte), timestampDelta (varint), offsetDelta
      // (varint)
      offset += 1;
      reader::read_signed_varint(buffer, offset);
      reader::read_signed_varint(buffer, offset);

      // Skip the key if it exists
      int32_t key_length = reader::read_signed_varint(buffer, offset);
      if (key_length > 0)
        offset += key_length;

      // 4. Extract the Value payload
      int32_t value_length = reader::read_signed_varint(buffer, offset);
      if (value_length > 0) {
        // KRaft ApiMessage Frame Header
        uint32_t frame_version = reader::read_unsigned_varint(buffer, offset);
        uint32_t record_type = reader::read_unsigned_varint(buffer, offset);
        uint32_t record_version = reader::read_unsigned_varint(buffer, offset);

        // Route to the correct Deserializer!
        if (record_type == 2) {
          auto topic =
              Deserializer<dto::TopicRecord>::deserialize(buffer, offset);
          cache.add_topic(topic);
        } else if (record_type == 3) {
          auto partition =
              Deserializer<dto::PartitionRecord>::deserialize(buffer, offset);
          cache.add_partition(partition);
        }
      }

      // FORCE alignment to the next record, skipping unknown schema fields &
      // headers!
      offset = record_end;
    }

    // FORCE alignment to the next batch
    offset = batch_end;
  }

  std::println("Cache # of topics: {}", cache.num_of_topics());
}
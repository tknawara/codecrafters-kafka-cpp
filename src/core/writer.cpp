#include "writer.hpp"

void kafka::writer::write_unsigned_varint(std::vector<uint8_t> &buffer,
                                          uint32_t value) {
  while (value > 0x7f) {
    buffer.push_back(static_cast<uint8_t>((value & 0x7f) | 0x80));
    value >>= 7;
  }
  buffer.push_back(static_cast<uint8_t>(value & 0x7f));
}

void kafka::writer::write_compact_string(std::vector<uint8_t> &buffer,
                                         const std::string &text) {
  uint32_t compact_length = text.size() + 1;
  write_unsigned_varint(buffer, compact_length);

  buffer.insert(buffer.end(), text.begin(), text.end());
}

void kafka::writer::write_nullable_compact_string(
    std::vector<uint8_t> &buffer, const std::optional<std::string> &value) {
  if (!value.has_value()) {
    // In Kafka compact types, a length of 0 explicitly means "null"
    write_unsigned_varint(buffer, 0);
  } else {
    // If it has a value, length is actual bytes + 1
    write_unsigned_varint(buffer, value->length() + 1);
    buffer.insert(buffer.end(), value->begin(), value->end());
  }
}

void kafka::writer::write_empty_tag_buffer(std::vector<uint8_t> &buffer) {
  write_unsigned_varint(buffer, 0);
}

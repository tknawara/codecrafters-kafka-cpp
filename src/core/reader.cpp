#include "reader.hpp"
#include <backward.hpp>
#include <stdexcept>

uint32_t kafka::reader::read_unsigned_varint(std::span<const uint8_t> buffer,
                                             size_t &offset) {
  uint32_t value = 0;
  uint32_t shift = 0;

  while (offset < buffer.size()) {
    uint8_t current_byte = buffer[offset];
    offset++;

    value |= static_cast<uint32_t>(current_byte & 0x7F) << shift;
    if ((current_byte & 0x80) == 0) {
      return value;
    }

    shift += 7;

    if (shift >= 32) {
      backward::StackTrace st;
      st.load_here(32);
      backward::Printer p;
      p.print(st, std::cerr);
      throw std::runtime_error("Malformed varint: exceeds 32 bits");
    }
  }

  backward::StackTrace st;
  st.load_here(32);
  backward::Printer p;
  p.print(st, std::cerr);
  throw std::out_of_range("Buffer ended before varint was fully parsed");
}

int32_t kafka::reader::read_signed_varint(std::span<const uint8_t> buffer,
                                          size_t &offset) {
  uint32_t raw = read_unsigned_varint(buffer, offset);

  // 2. ZigZag decode: shift right by 1, and XOR with the sign bit
  int32_t value = (raw >> 1) ^ -(raw & 1);

  return value;
}

std::string kafka::reader::read_compact_string(std::span<const uint8_t> buffer,
                                               size_t &offset) {
  uint32_t compact_length = read_unsigned_varint(buffer, offset);
  if (compact_length == 0) {
    return "";
  }

  size_t actual_length = compact_length - 1;

  std::string result(buffer.begin() + offset,
                     buffer.begin() + offset + actual_length);

  offset += actual_length;
  return result;
}

std::optional<std::string>
kafka::reader::read_nullable_compact_string(std::span<const uint8_t> buffer,
                                            size_t &offset) {
  uint32_t compact_length = read_unsigned_varint(buffer, offset);
  if (compact_length == 0) {
    return std::nullopt; // Explicitly null
  }
  size_t actual_length = compact_length - 1;
  std::string result(buffer.begin() + offset,
                     buffer.begin() + offset + actual_length);
  offset += actual_length;
  return result;
}

std::optional<std::vector<uint8_t>>
kafka::reader::read_compact_bytes(std::span<const uint8_t> buffer,
                                  size_t &offset) {
  uint32_t compact_length = read_unsigned_varint(buffer, offset);
  if (compact_length == 0) {
    return std::nullopt;
  }
  size_t actual_length = compact_length - 1;
  std::vector<uint8_t> result(buffer.begin() + offset,
                              buffer.begin() + offset + actual_length);
  offset += actual_length;
  return result;
}

uint32_t
kafka::reader::read_compact_array_length(std::span<const uint8_t> buffer,
                                         size_t &offset) {
  uint32_t compact_length = read_unsigned_varint(buffer, offset);
  return (compact_length == 0) ? 0 : compact_length - 1;
}

void kafka::reader::skip_tag_buffer(std::span<const uint8_t> buffer,
                                    size_t &offset) {
  uint32_t tags_count = read_unsigned_varint(buffer, offset);
  for (uint32_t i = 0; i < tags_count; ++i) {
    read_unsigned_varint(buffer, offset); // Skip tag type
    uint32_t tag_length =
        read_unsigned_varint(buffer, offset); // Read tag length
    offset += tag_length;                     // Jump over the tag payload
  }
}
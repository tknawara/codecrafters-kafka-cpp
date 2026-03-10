#include "reader.hpp"
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
      throw std::runtime_error("Malformed varint: exceeds 32 bits");
    }
  }

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
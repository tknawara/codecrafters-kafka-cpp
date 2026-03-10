#pragma once

#include <bit>
#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <vector>

#include "core/deserializable.hpp"

namespace kafka::reader {

uint32_t read_unsigned_varint(std::span<const uint8_t> buffer, size_t &offset);
int32_t read_signed_varint(std::span<const uint8_t> buffer, size_t &offset);
std::string read_compact_string(std::span<const uint8_t> buffer,
                                size_t &offset);

template <typename T>
T read_be(std::span<const uint8_t> buffer, size_t &offset) {
  T value = 0;
  std::memcpy(&value, buffer.data() + offset, sizeof(T));
  offset += sizeof(T);
  return std::byteswap(value);
}

template <Deserializable T>
std::vector<T> read_compact_array(std::span<const uint8_t> buffer,
                                  size_t &offset) {
  uint32_t compact_length = read_unsigned_varint(buffer, offset);

  if (compact_length == 0) {
    return {};
  }

  size_t actual_length = compact_length - 1;

  std::vector<T> result;
  result.reserve(actual_length);

  for (size_t i = 0; i < actual_length; ++i) {
    result.push_back(Deserializer<T>::deserialize(buffer, offset));
  }

  return result;
}

}; // namespace kafka::reader
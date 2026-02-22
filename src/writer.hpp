#pragma once

#include <bit>
#include <cstdint>
#include <cstring>
#include <vector>

#include "serializable.hpp"

namespace kafka::writer {

void write_unsigned_varint(std::vector<uint8_t> &buffer, uint32_t value);

template <typename T> void write_be(std::vector<uint8_t> &buffer, T value) {
  T swapped_value = std::byteswap(value);
  const uint8_t *byte_ptr = reinterpret_cast<const uint8_t *>(&swapped_value);
  buffer.insert(buffer.end(), byte_ptr, byte_ptr + sizeof(T));
}

template <Serializable S>
void write_compact_array(std::vector<uint8_t> &buffer,
                         const std::vector<S> &items) {
  uint32_t compact_length = items.size() + 1;
  write_unsigned_varint(buffer, compact_length);

  for (const auto &item : items) {
    serialize(buffer, item);
    buffer.push_back(0);
  }
}

}; // namespace kafka::writer
#pragma once

#include <bit>
#include <cstdint>
#include <cstring>
#include <vector>

namespace kafka::writer {
template <typename T> void write_be(std::vector<uint8_t> &buffer, T value) {
  T swapped_value = std::byteswap(value);
  const uint8_t *byte_ptr = reinterpret_cast<const uint8_t *>(&swapped_value);
  buffer.insert(buffer.end(), byte_ptr, byte_ptr + sizeof(T));
}
}; // namespace kafka::writer
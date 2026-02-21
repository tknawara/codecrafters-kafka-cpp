#pragma once

#include <bit>
#include <cstdint>
#include <cstring>
#include <vector>

namespace kafka::reader {
template <typename T>
T read_be(const std::vector<uint8_t> &buffer, size_t &offset) {
  T value = 0;
  std::memcpy(&value, buffer.data() + offset, sizeof(T));
  offset += sizeof(T);
  return std::byteswap(value);
}
}; // namespace kafka::reader
#pragma once

#include "core/deserializable.hpp"
#include "core/reader.hpp"

namespace kafka {

template <> struct Deserializer<int32_t> {
  static int32_t deserialize(std::span<const uint8_t> buffer, size_t &offset) {
    int32_t value = reader::read_be<int32_t>(buffer, offset);
    return value;
  }
};

template <> struct Deserializer<std::string> {
  static std::string deserialize(std::span<const uint8_t> buffer,
                                 size_t &offset) {
    return reader::read_compact_string(buffer, offset);
  }
};
} // namespace kafka

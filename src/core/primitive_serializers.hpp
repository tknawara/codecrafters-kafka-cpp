#pragma once

#include "core/serializable.hpp"
#include "core/writer.hpp"

namespace kafka {
template <> struct Serializer<int32_t> {
  static void serialize(std::vector<uint8_t> &buffer, int32_t value) {
    writer::write_be(buffer, value);
  }
};

template <> struct Serializer<int16_t> {
  static void serialize(std::vector<uint8_t> &buffer, int16_t value) {
    writer::write_be(buffer, value);
  }
};

template <> struct Serializer<std::string> {
  static void serialize(std::vector<uint8_t> &buffer,
                        const std::string &value) {
    writer::write_compact_string(buffer, value);
  }
};
} // namespace kafka
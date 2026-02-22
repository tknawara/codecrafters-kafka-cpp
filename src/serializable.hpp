#pragma once

#include <concepts>
#include <inttypes.h>
#include <vector>

namespace kafka {
template <typename T>
concept Serializable = requires(const T &item, std::vector<uint8_t> &buffer) {
  // Enforce that a free function exists and is callable
  { serialize(buffer, item) } -> std::same_as<void>;
};
} // namespace kafka
#pragma once

#include <concepts>
#include <inttypes.h>
#include <vector>

namespace kafka {
template <typename T>
concept Serializable = requires(std::vector<uint8_t> &buffer, const T &item) {
  // Enforce that a free function exists and is callable
  { serialize(buffer, item) } -> std::same_as<void>;
};
} // namespace kafka
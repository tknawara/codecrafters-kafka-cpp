#pragma once

#include <cstdint>
#include <span>

namespace kafka {
template <typename T> struct Deserializer;

template <typename T>
concept Deserializable =
    requires(std::span<const uint8_t> buffer, size_t offset) {
      { Deserializer<T>::deserialize(buffer, offset) } -> std::same_as<T>;
    };
}; // namespace kafka
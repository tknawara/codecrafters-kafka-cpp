#pragma once

#include <concepts>
#include <inttypes.h>
#include <vector>

namespace kafka {
template <typename T> struct Serializer;

template <typename T>
concept Serializable = requires(std::vector<uint8_t> &buffer, const T &item) {
  { Serializer<T>::serialize(buffer, item) } -> std::same_as<void>;
};

}; // namespace kafka
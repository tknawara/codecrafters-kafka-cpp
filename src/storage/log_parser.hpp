#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace kafka::storage {

auto extract_records_from_offset(std::span<const uint8_t> log_bytes,
                                 int64_t target_offset) -> std::vector<uint8_t>;

} // namespace kafka::storage
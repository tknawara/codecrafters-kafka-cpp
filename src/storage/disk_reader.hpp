#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace kafka::storage {

auto read_log_file(const std::string &topic_name, int32_t partition_index)
    -> std::optional<std::vector<uint8_t>>;

} // namespace kafka::storage
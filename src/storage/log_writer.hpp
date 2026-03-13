#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace kafka::storage {

// Returns true if the write was successful
bool append_to_log(const std::string &topic_name, int32_t partition_index,
                   const std::vector<uint8_t> &records);

} // namespace kafka::storage
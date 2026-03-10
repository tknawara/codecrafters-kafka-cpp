#pragma once

#include "metadata/cache.hpp"

namespace kafka::metadata {

class MetadataParser {
public:
  static auto parse_log_file(const std::string &filepath) -> MetadataCache;
};

} // namespace kafka::metadata
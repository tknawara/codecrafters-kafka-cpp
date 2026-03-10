#pragma once

#include "metadata/cache.hpp"

namespace kafka::metadata {

class MetadataParser {
public:
  static void parse_log_file(const std::string &filepath, MetadataCache &cache);
};

} // namespace kafka::metadata
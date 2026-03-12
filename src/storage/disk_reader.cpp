#include "disk_reader.hpp"
#include <format>
#include <fstream>

namespace kafka::storage {

auto read_log_file(const std::string &topic_name, int32_t partition_index)
    -> std::optional<std::vector<uint8_t>> {
  std::string file_path =
      std::format("/tmp/kraft-combined-logs/{}-{}/00000000000000000000.log",
                  topic_name, partition_index);

  std::ifstream file(file_path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    return std::nullopt;
  }
  std::streamsize file_size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> buffer(file_size);
  if (file.read(reinterpret_cast<char *>(buffer.data()), file_size)) {
    return buffer;
  }

  return std::nullopt;
}

} // namespace kafka::storage
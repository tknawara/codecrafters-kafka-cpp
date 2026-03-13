#include "log_writer.hpp"
#include <filesystem>
#include <format>
#include <fstream>

namespace kafka::storage {

bool append_to_log(const std::string &topic_name, int32_t partition_index,
                   const std::vector<uint8_t> &records) {
  if (records.empty()) {
    return true; // Nothing to write
  }

  std::string dir_path = std::format("/tmp/kraft-combined-logs/{}-{}",
                                     topic_name, partition_index);
  std::string file_path = std::format("{}/00000000000000000000.log", dir_path);

  // 1. Ensure the directory exists before attempting to open a file inside it
  std::filesystem::create_directories(dir_path);

  // 2. Open the file in append and binary mode
  std::ofstream file(file_path, std::ios::binary | std::ios::app);
  if (!file.is_open()) {
    return false;
  }

  // 3. Dump the raw bytes straight to disk
  file.write(reinterpret_cast<const char *>(records.data()), records.size());

  return file.good();
}

} // namespace kafka::storage
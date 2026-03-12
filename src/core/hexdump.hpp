#pragma once

#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace hexdump {

struct Style {
  static constexpr auto reset = "\033[0m";
  static constexpr auto dim = "\033[2m";
  static constexpr auto cyan = "\033[36m";
  static constexpr auto yellow = "\033[33m";
  static constexpr auto green = "\033[32m";
  static constexpr auto magenta = "\033[35m";
  static constexpr auto white = "\033[97m";
  static constexpr auto red = "\033[31m";
};

// Dump a span of bytes in classic hex+ASCII format to the given stream.
//   00000000  48 65 6c 6c 6f 20 57 6f  72 6c 64 21 0a 00 ff fe  |Hello Wo
//   rld!....|
//
// Options:
//   title       - optional header label (e.g. "metadata.log")
//   bytes_per_row - 16 is standard, 8 for narrow terminals
void dump(std::span<const uint8_t> data, std::ostream &out = std::cerr,
          const std::string &title = "", size_t bytes_per_row = 16);

// Convenience: dump from a vector
inline void dump(const std::vector<uint8_t> &data,
                 std::ostream &out = std::cerr, const std::string &title = "",
                 size_t bytes_per_row = 16) {
  dump(std::span<const uint8_t>(data), out, title, bytes_per_row);
}

// Convenience: dump from an ifstream (reads entire file)
void dump_file(const std::string &path, std::ostream &out = std::cerr,
               const std::string &title = "", size_t bytes_per_row = 16);

} // namespace hexdump
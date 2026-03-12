#include "hexdump.hpp"

#include <algorithm>
#include <format>
#include <fstream>

namespace hexdump {

static bool is_printable(uint8_t byte) { return byte >= 0x20 && byte <= 0x7E; }

// Calculate the visual inner width of a row (between the outer │ chars)
// " 00000000  xx xx xx xx xx xx xx xx  xx xx xx xx xx xx xx xx  │
// xxxxxxxxxxxxxxxx "
static size_t inner_width(size_t bytes_per_row) {
  // space(1) + offset(8) + spaces(2)
  // + hex(bytes_per_row * 3) + mid_gaps(bytes_per_row / 8 - 1)
  // + space(1) + pipe(1) + space(1)
  // + ascii(bytes_per_row) + space(1)
  size_t hex_section = bytes_per_row * 3 + (bytes_per_row / 8 - 1);
  return 1 + 8 + 2 + hex_section + 1 + 1 + 1 + bytes_per_row + 1;
}

static std::string h_line(size_t width) {
  std::string line;
  for (size_t i = 0; i < width; ++i)
    line += "\u2500"; // ─
  return line;
}

void dump(std::span<const uint8_t> data, std::ostream &out,
          const std::string &title, size_t bytes_per_row) {

  size_t width = inner_width(bytes_per_row);

  // Top border
  if (!title.empty()) {
    std::string label = std::format(" {}{}{} ", Style::cyan, title, Style::dim);
    size_t remaining = width - (title.size() + 3);
    out << std::format("{}\u250c{}{}{}\u2510{}\n", Style::dim, "\u2500", label,
                       h_line(remaining), Style::reset);
  } else {
    out << std::format("{}\u250c{}\u2510{}\n", Style::dim, h_line(width),
                       Style::reset);
  }

  // Column header
  out << std::format("{}\u2502{} {:<10s}", Style::dim, Style::reset, "Offset");
  for (size_t i = 0; i < bytes_per_row; ++i) {
    if (i > 0 && i % 8 == 0)
      out << " ";
    out << std::format("{}{:02X}{} ", Style::dim, i, Style::reset);
  }
  out << std::format(" {}\u2502{} ", Style::dim, Style::reset);
  out << "ASCII";
  out << std::string(bytes_per_row - 5, ' ');
  out << std::format(" {}\u2502{}\n", Style::dim, Style::reset);

  // Separator
  out << std::format("{}\u251c{}\u2524{}\n", Style::dim, h_line(width),
                     Style::reset);

  // Data rows
  for (size_t row_offset = 0; row_offset < data.size();
       row_offset += bytes_per_row) {
    out << std::format("{}\u2502{} {}{:08x}{}  ", Style::dim, Style::reset,
                       Style::yellow, row_offset, Style::reset);

    size_t row_end = std::min(row_offset + bytes_per_row, data.size());
    size_t row_len = row_end - row_offset;

    // Hex bytes
    for (size_t i = 0; i < bytes_per_row; ++i) {
      if (i > 0 && i % 8 == 0)
        out << " ";
      if (i < row_len) {
        uint8_t byte = data[row_offset + i];
        if (byte == 0x00) {
          out << std::format("{}{:02x}{} ", Style::dim, byte, Style::reset);
        } else if (is_printable(byte)) {
          out << std::format("{}{:02x}{} ", Style::green, byte, Style::reset);
        } else {
          out << std::format("{}{:02x}{} ", Style::white, byte, Style::reset);
        }
      } else {
        out << "   ";
      }
    }

    // ASCII column
    out << std::format(" {}\u2502{} ", Style::dim, Style::reset);
    for (size_t i = 0; i < bytes_per_row; ++i) {
      if (i < row_len) {
        uint8_t byte = data[row_offset + i];
        if (is_printable(byte)) {
          out << std::format("{}{}{}", Style::green, static_cast<char>(byte),
                             Style::reset);
        } else {
          out << std::format("{}\u00b7{}", Style::dim, Style::reset);
        }
      } else {
        out << " ";
      }
    }
    out << std::format(" {}\u2502{}\n", Style::dim, Style::reset);
  }

  // Bottom border
  std::string summary =
      std::format(" {}{} bytes{} ", Style::magenta, data.size(), Style::dim);
  size_t remaining = width - (std::format("{} bytes", data.size()).size() + 3);
  out << std::format("{}\u2514{}{}{}\u2518{}\n\n", Style::dim, "\u2500",
                     summary, h_line(remaining), Style::reset);
}

void dump_file(const std::string &path, std::ostream &out,
               const std::string &title, size_t bytes_per_row) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    out << std::format("{}[hexdump]{} Failed to open: {}\n", Style::red,
                       Style::reset, path);
    return;
  }

  std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

  std::string label = title.empty() ? path : title;
  dump(data, out, label, bytes_per_row);
}

} // namespace hexdump
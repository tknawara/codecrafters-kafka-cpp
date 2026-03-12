#include "log_parser.hpp"
#include "core/reader.hpp"

auto kafka::storage::extract_records_from_offset(
    std::span<const uint8_t> log_bytes, int64_t target_offset)
    -> std::vector<uint8_t> {
  size_t current_pos = 0;
  std::vector<uint8_t> result;

  while (current_pos + 12 <= log_bytes.size()) {
    size_t batch_start = current_pos;

    int64_t base_offset = reader::read_be<int64_t>(log_bytes, current_pos);
    int32_t batch_length = reader::read_be<int32_t>(log_bytes, current_pos);

    size_t total_batch_size = 12 + batch_length;

    if (batch_start + total_batch_size > log_bytes.size()) {
      break;
    }

    if (base_offset >= target_offset || !result.empty()) {
      auto batch_span = log_bytes.subspan(batch_start, total_batch_size);
      result.insert(result.end(), batch_span.begin(), batch_span.end());
    }

    current_pos = batch_start + total_batch_size;
  }

  return result;
}

#include <bit>
#include <cstring>

#include "response.hpp"

std::vector<uint8_t> kafka::response::serialize(const Response &response) {
  uint32_t message_size_raw = std::byteswap(10);
  uint32_t correlation_id_raw = std::byteswap(response.correlation_id);
  uint16_t error_code_raw =
      std::byteswap(static_cast<uint16_t>(response.error));

  std::vector<uint8_t> ret(10);
  std::memcpy(&ret[0], &message_size_raw, 4);
  std::memcpy(&ret[4], &correlation_id_raw, 4);
  std::memcpy(&ret[8], &error_code_raw, 2);

  return ret;
}
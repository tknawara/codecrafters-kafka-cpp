#include <bit>
#include <cstring>

#include "request.hpp"

kafka::request::Request kafka::request::from_bytes(uint32_t message_size,
                                                   std::vector<uint8_t> raw) {
  uint16_t api_raw;
  uint16_t version_raw;
  uint32_t correlation_id_raw;

  std::memcpy(&api_raw, &raw[0], 2);
  std::memcpy(&version_raw, &raw[2], 2);
  std::memcpy(&correlation_id_raw, &raw[4], 4);

  Header header{.api = static_cast<KafkaApi>(std::byteswap(api_raw)),
                .version = std::byteswap(version_raw),
                .correlation_id = std::byteswap(correlation_id_raw)};

  raw.erase(raw.begin(), raw.begin() + 8);

  Request request{message_size, header, raw};
  return request;
}
#include "request.hpp"
#include "reader.hpp"

kafka::request::Request kafka::request::from_bytes(uint32_t message_size,
                                                   std::vector<uint8_t> raw) {

  size_t offset = 0;

  auto api_raw = reader::read_be<uint16_t>(raw, offset);
  auto api_key = static_cast<api::metadata::ApiKey>(api_raw);
  auto api_version = reader::read_be<uint16_t>(raw, offset);
  auto correlation_id = reader::read_be<uint32_t>(raw, offset);

  Header header{api_key, api_version, correlation_id};

  raw.erase(raw.begin(), raw.begin() + offset);

  Request request{message_size, header, raw};
  return request;
}
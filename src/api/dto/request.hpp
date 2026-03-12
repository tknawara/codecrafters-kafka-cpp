#pragma once

#include <inttypes.h>
#include <vector>

#include "api/registry.hpp"
#include "core/deserializable.hpp"
#include "core/reader.hpp"

namespace kafka::api::dto {

struct RequestHeader {
  registry::ApiKey api;
  uint16_t version;
  uint32_t correlation_id;
  // Note: Depending on the request API version, Kafka also sends a
  // nullable string 'client_id' and a 'TAG_BUFFER' right here.
};

struct Request {

  uint32_t message_size;
  RequestHeader header;
  std::vector<uint8_t> body;
};

}; // namespace kafka::api::dto

namespace kafka {

template <> struct Deserializer<api::dto::RequestHeader> {
  static api::dto::RequestHeader deserialize(std::span<const uint8_t> buffer,
                                             size_t &offset) {
    api::dto::RequestHeader header;

    int16_t raw_api = reader::read_be<int16_t>(buffer, offset);
    header.api = static_cast<api::registry::ApiKey>(raw_api);

    header.version = reader::read_be<uint16_t>(buffer, offset);
    header.correlation_id = reader::read_be<uint32_t>(buffer, offset);

    // 1. Skip the nullable client_id string
    int16_t client_id_length = reader::read_be<int16_t>(buffer, offset);
    if (client_id_length > 0) {
      offset += client_id_length;
    }

    auto header_version =
        api::registry::get_request_header_version(header.api, header.version);

    if (header_version == 2) {
      // Flexible header v2 has a TAG_BUFFER.
      // Safely read the LEB128 varint so the offset advances correctly!
      uint32_t tag_length = reader::read_unsigned_varint(buffer, offset);
    }

    return header;
  }
};

template <> struct Deserializer<api::dto::Request> {
  static api::dto::Request deserialize(std::span<const uint8_t> buffer,
                                       size_t &offset) {
    api::dto::Request req;
    req.header =
        Deserializer<api::dto::RequestHeader>::deserialize(buffer, offset);
    size_t body_size = buffer.size() - offset;
    req.body = std::vector<uint8_t>(buffer.begin() + offset,
                                    buffer.begin() + offset + body_size);
    offset += body_size;
    return req;
  }
};

} // namespace kafka
#include "handler.hpp"

#include <stdexcept>

kafka::response::Response
kafka::Handler::handle(const kafka::request::Request &request) {
  switch (request.header.api) {
  case kafka::request::KafkaApi::ApiVersions:
    return handle_api_versions(request);
  default:
    throw std::invalid_argument("unsupported api");
  }
}

kafka::response::Response
kafka::Handler::handle_api_versions(const kafka::request::Request &request) {
  response::ApiVersionsResponse body{};
  body.error = error::ErrorCode::None;
  body.keys.push_back({.api_key = static_cast<uint16_t>(request.header.api),
                       .min_version = 0,
                       .max_version = 4});
  if (!supported_version(request.header.version)) {
    body.error = error::ErrorCode::UnsupportedVersion;
    body.keys.clear();
  }

  response::Response response{
      .correlation_id = request.header.correlation_id,
      .body = body,
  };
  return response;
}

bool kafka::Handler::supported_version(uint16_t version) {
  return version >= 0 && version <= 4;
}
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
  response::Response response{};
  response.correlation_id = request.header.correlation_id;
  if (!supported_version(request.header.version)) {
    response.error = error::ErrorCode::UnsupportedVersion;
  }
  return response;
}

bool kafka::Handler::supported_version(uint16_t version) {
  return version >= 0 && version <= 4;
}
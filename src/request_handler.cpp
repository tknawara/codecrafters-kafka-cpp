#include "request_handler.hpp"

#include <stdexcept>

#include "api_details.hpp"

auto kafka::RequestHandler::handle(const kafka::request::Request &request)
    -> api::dto::Response {
  switch (request.header.api) {
  case kafka::api::metadata::ApiKey::ApiVersions:
    return handle_api_versions(request);
  default:
    throw std::invalid_argument("unsupported api");
  }
}

auto kafka::RequestHandler::handle_api_versions(
    const kafka::request::Request &request) -> api::dto::Response {
  api::dto::ApiVersionsResponse body{};
  body.error = error::ErrorCode::None;
  for (const auto api_detail : api::metadata::get_all_api_details()) {
    body.keys.push_back(api_detail);
  }
  if (!supported_version(request.header.version)) {
    body.error = error::ErrorCode::UnsupportedVersion;
    body.keys.clear();
  }

  api::dto::Response response{
      .correlation_id = request.header.correlation_id,
      .body = body,
  };
  return response;
}

bool kafka::RequestHandler::supported_version(uint16_t version) {
  return version >= 0 && version <= 4;
}
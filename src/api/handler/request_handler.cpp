#include "request_handler.hpp"

#include <stdexcept>

#include "api/registry.hpp"

auto kafka::RequestHandler::handle(const api::dto::Request &request)
    -> api::dto::Response {
  switch (request.header.api) {
  case api::registry::ApiKey::ApiVersions:
    return handle_api_versions(request);
  case api::registry::ApiKey::DescribeTopicParititons:
    return handle_describe_topic_partitions(request);
  default:
    throw std::invalid_argument("unsupported api");
  }
}

auto kafka::RequestHandler::handle_api_versions(
    const api::dto::Request &request) -> api::dto::Response {
  api::dto::ApiVersionsResponse body{};
  body.error = error::ErrorCode::None;
  for (const auto api_detail : api::dto::get_all_api_details()) {
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

auto kafka::RequestHandler::handle_describe_topic_partitions(
    const api::dto::Request &request) -> api::dto::Response {
  std::span<const uint8_t> body_span{request.body};
  size_t offset = 0;

  auto req_body =
      Deserializer<api::dto::DescribeTopicPartitionsRequest>::deserialize(
          body_span, offset);

  api::dto::DescribeTopicPartitionsResponse res_body{};
  res_body.throttle_time_ms = 0;
  res_body.next_cursor = -1;

  for (const auto &req_topic : req_body.topics) {
    api::dto::DescribeTopicPartitionsResponseTopic res_topic{};

    res_topic.name = req_topic.topic_name;

    res_topic.error_code = 3;

    // Defaults from our DTO:
    // is_internal = false
    // topic_authorized_operations = 0x00000df8
    // topic_id = 16 empty bytes
    // partitions = empty array

    res_body.topics.push_back(res_topic);
  }

  api::dto::Response response{
      .correlation_id = request.header.correlation_id,
      .body = res_body,
  };

  return response;
}

bool kafka::RequestHandler::supported_version(uint16_t version) {
  return version <= 4;
}
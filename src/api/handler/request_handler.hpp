#pragma once

#include "api/dto/request.hpp"
#include "api/dto/response.hpp"

namespace kafka {

class RequestHandler {
public:
  auto handle(const api::dto::Request &request) -> api::dto::Response;

private:
  auto handle_api_versions(const api::dto::Request &request)
      -> api::dto::Response;
  auto handle_describe_topic_partitions(const api::dto::Request &request)
      -> api::dto::Response;
  bool supported_version(uint16_t version);
};

}; // namespace kafka
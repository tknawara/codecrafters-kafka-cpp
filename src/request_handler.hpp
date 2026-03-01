#pragma once

#include "request.hpp"

namespace kafka {

class RequestHandler {
public:
  auto handle(const request::Request &request) -> api::dto::Response;

private:
  auto handle_api_versions(const request::Request &request)
      -> api::dto::Response;
  bool supported_version(uint16_t version);
};

}; // namespace kafka
#pragma once

#include "request.hpp"
#include "response.hpp"

namespace kafka {

class Handler {
public:
  response::Response handle(const request::Request &request);

private:
  response::Response handle_api_versions(const request::Request &request);
  bool supported_version(uint16_t version);
};

}; // namespace kafka
#pragma once

#include "api/dto/request.hpp"
#include "api/dto/response.hpp"
#include "api/types.hpp"

namespace kafka::api::middleware {
kafka::api::dto::Response
version_validator_middleware(const api::dto::Request &request,
                             HandlerFunc next);

kafka::api::dto::Response logging_middleware(const api::dto::Request &request,
                                             HandlerFunc next);
} // namespace kafka::api::middleware
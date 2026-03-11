#pragma once

#include <functional>

#include "api/dto/request.hpp"
#include "api/dto/response.hpp"

namespace kafka {
using HandlerFunc =
    std::function<api::dto::Response(const api::dto::Request &request)>;

using MiddlewareFunc = std::function<api::dto::Response(
    const api::dto::Request &request, HandlerFunc)>;
} // namespace kafka

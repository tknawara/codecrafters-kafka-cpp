#include "api/middleware.hpp"

#include <chrono>
#include <print>
#include <string>

#include "api/dto/api_versions.hpp"
#include "api/dto/describe_topic_partitions.hpp"
#include "api/dto/fetch_partitions.hpp"
#include "api/registry.hpp"

kafka::api::dto::Response kafka::api::middleware::version_validator_middleware(
    const api::dto::Request &request, HandlerFunc next) {
  if (!api::dto::is_supported_version(request.header.api,
                                      request.header.version)) {
    std::println("[Pipeline] Rejecting unsupported version!");
    switch (request.header.api) {
    case api::registry::ApiKey::ApiVersions: {
      api::dto::ApiVersionsResponse res_body{};
      res_body.error_code = 35;
      return api::dto::Response{
          .correlation_id = request.header.correlation_id,
          .body = res_body,
      };
    }
    case api::registry::ApiKey::DescribeTopicParititons: {
      api::dto::DescribeTopicPartitionsResponse res_body{};
      return api::dto::Response{
          .correlation_id = request.header.correlation_id,
          .body = res_body,
      };
    }
    case api::registry::ApiKey::Fetch: {
      api::dto::FetchResponse res_body{};
      res_body.error_code = 35;
      return api::dto::Response{
          .correlation_id = request.header.correlation_id,
          .body = res_body,
      };
    }
    default:
      throw std::invalid_argument("Cannot generate error for unknown API");
    }
  }

  return next(request);
}

constexpr auto color_reset = "\033[0m";
constexpr auto color_cyan = "\033[36m";    // For incoming requests
constexpr auto color_green = "\033[32m";   // For successful responses
constexpr auto color_yellow = "\033[33m";  // For timing
constexpr auto color_magenta = "\033[35m"; // For the middleware tag

static constexpr std::string get_api_name(kafka::api::registry::ApiKey key) {
  switch (key) {
  case kafka::api::registry::ApiKey::ApiVersions:
    return "ApiVersions";
  case kafka::api::registry::ApiKey::DescribeTopicParititons:
    return "DescribeTopicPartitions";
  case kafka::api::registry::ApiKey::Fetch:
    return "Fetch";
  default:
    return "Unknown";
  }
}

kafka::api::dto::Response
kafka::api::middleware::logging_middleware(const api::dto::Request &request,
                                           HandlerFunc next) {
  // 1. Start the timer
  auto start_time = std::chrono::high_resolution_clock::now();

  // 2. Log the incoming request (Cyan)
  std::println(
      "{}[Router]{} {}-> INCOMING{}  | API: {} (v{}), Correlation ID: {}",
      color_magenta, color_reset, color_cyan, color_reset,
      get_api_name(request.header.api), request.header.version,
      request.header.correlation_id);

  // 3. Pass control down the onion to your controller
  api::dto::Response response = next(request);

  // 4. Stop the timer
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(
                         end_time - start_time)
                         .count();

  // 5. Log the outgoing response (Green + Yellow for timing)
  std::println("{}[Router]{} {}<- OUTGOING{}  | API: {}, Correlation ID: {} "
               "{}[Took: {} us]{}",
               color_magenta, color_reset, color_green, color_reset,
               get_api_name(request.header.api), response.correlation_id,
               color_yellow, duration_us, color_reset);

  return response;
}
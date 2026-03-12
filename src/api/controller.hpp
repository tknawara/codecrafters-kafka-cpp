#pragma once

#include "api/dto/request.hpp"
#include "api/dto/response.hpp"
#include "metadata/cache.hpp"

namespace kafka {

class KafkaController {
public:
  explicit KafkaController(const metadata::MetadataCache &cache)
      : cache_(cache) {}
  auto handle(const api::dto::Request &request) -> api::dto::Response;

private:
  const metadata::MetadataCache &cache_;

  auto handle_api_versions(const api::dto::Request &request)
      -> api::dto::Response;
  auto handle_describe_topic_partitions(const api::dto::Request &request)
      -> api::dto::Response;
  auto handle_fetch_partitions(const api::dto::Request &request)
      -> api::dto::Response;
  bool supported_version(uint16_t version);
};

}; // namespace kafka
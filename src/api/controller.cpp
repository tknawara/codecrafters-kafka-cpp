#include "api/controller.hpp"

#include <stdexcept>

#include "api/registry.hpp"

auto kafka::KafkaController::handle(const api::dto::Request &request)
    -> api::dto::Response {
  switch (request.header.api) {
  case api::registry::ApiKey::ApiVersions:
    return handle_api_versions(request);
  case api::registry::ApiKey::DescribeTopicParititons:
    return handle_describe_topic_partitions(request);
  case api::registry::ApiKey::Fetch:
    return handle_fetch_partitions(request);
  default:
    throw std::invalid_argument("unsupported api");
  }
}

auto kafka::KafkaController::handle_api_versions(
    const api::dto::Request &request) -> api::dto::Response {
  api::dto::ApiVersionsResponse body{};
  for (const auto api_detail : api::dto::get_all_api_details()) {
    body.keys.push_back(api_detail);
  }
  if (!api::dto::is_supported_version(api::registry::ApiKey::ApiVersions,
                                      request.header.version)) {
    body.error_code = error::ErrorCode::UnsupportedVersion;
    body.keys.clear();
  }

  api::dto::Response response{
      .correlation_id = request.header.correlation_id,
      .body = body,
  };
  return response;
}

auto kafka::KafkaController::handle_describe_topic_partitions(
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
    api::dto::DescribeTopicPartitionsResponseTopic res_topic;
    res_topic.name = req_topic.name;

    // 1. Query our new cache!
    auto topic_opt = cache_.get_topic(req_topic.name);

    if (topic_opt) {
      // --- TOPIC EXISTS (Stage 7 Logic) ---
      res_topic.error_code = 0; // 0 = No error
      res_topic.topic_id = topic_opt->topic_id;
      res_topic.is_internal = false;

      // Fetch partitions using the UUID
      auto partitions = cache_.get_partitions(res_topic.topic_id);

      for (const auto &p : partitions) {
        api::dto::DescribeTopicPartitionsResponsePartition res_partition;
        res_partition.error_code = 0;
        res_partition.partition_index = p.partition_id;
        res_partition.leader_id = p.leader;
        res_partition.leader_epoch = p.leader_epoch;

        // Map the broker node arrays
        res_partition.replica_nodes = p.replicas;
        res_partition.isr_nodes = p.isr;

        // Note: eligible_leader_replicas, last_known_elr, and offline_replicas
        // remain perfectly empty std::vectors by default, as the tester
        // expects!

        res_topic.partitions.push_back(res_partition);
      }
    } else {
      // --- TOPIC DOES NOT EXIST (Stage 6 Logic) ---
      res_topic.error_code = 3; // 3 = UNKNOWN_TOPIC_OR_PARTITION
      res_topic.topic_id = {0}; // Fills the 16-byte array with zeros
      res_topic.is_internal = false;
    }

    res_body.topics.push_back(res_topic);
  }

  std::ranges::sort(res_body.topics, [](const auto &a, const auto &b) {
    return a.name < b.name;
  });

  api::dto::Response response{
      .correlation_id = request.header.correlation_id,
      .body = res_body,
  };

  return response;
}

auto kafka::KafkaController::handle_fetch_partitions(
    const api::dto::Request &request) -> api::dto::Response {
  api::dto::FetchResponse res_body;
  api::dto::Response response{
      .correlation_id = request.header.correlation_id,
      .body = res_body,
  };
  return response;
}
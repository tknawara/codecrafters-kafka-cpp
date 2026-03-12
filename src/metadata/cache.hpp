#pragma once

#include <array>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "metadata/dto/topic.hpp"

namespace kafka::metadata {

class MetadataParser;
class MetadataCache {
  friend class MetadataParser;

private:
  std::unordered_map<std::string, dto::TopicRecord> topics_by_name;
  std::map<std::array<uint8_t, 16>, std::vector<dto::PartitionRecord>>
      partitions_by_id;

  MetadataCache() = default;

  void add_topic(const dto::TopicRecord &record) {
    topics_by_name[record.name] = record;
  }

  void add_partition(const dto::PartitionRecord &record) {
    partitions_by_id[record.topic_id].push_back(record);
  }

public:
  size_t num_of_topics() const { return topics_by_name.size(); }

  std::optional<dto::TopicRecord> get_topic(const std::string &name) const {
    if (auto it = topics_by_name.find(name); it != topics_by_name.end()) {
      return it->second;
    }
    return std::nullopt;
  }

  std::optional<dto::TopicRecord>
  get_topic_by_id(const std::array<uint8_t, 16> &target_topic_id) const {
    auto it = std::ranges::find_if(
        topics_by_name,
        [&target_topic_id](const auto &topic_id) {
          return topic_id == target_topic_id;
        },
        [](const auto &p) { return p.second.topic_id; });

    if (it == topics_by_name.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  std::vector<dto::PartitionRecord>
  get_partitions(const std::array<uint8_t, 16> &topic_id) const {
    if (auto it = partitions_by_id.find(topic_id);
        it != partitions_by_id.end()) {
      return it->second;
    }
    return {};
  }
};

} // namespace kafka::metadata
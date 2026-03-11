#include "api_versions.hpp"

#include <algorithm>

auto kafka::api::dto::get_api_details(uint16_t raw_key)
    -> std::optional<ApiDetails> {
  auto match =
      std::ranges::find_if(supported_apis, [raw_key](const ApiDetails &api) {
        return static_cast<uint16_t>(api.key) == raw_key;
      });

  if (match != supported_apis.end()) {
    return *match;
  }

  return std::nullopt;
}

auto kafka::api::dto::get_all_api_details() -> std::vector<ApiDetails> {
  return std::vector<ApiDetails>(supported_apis.begin(), supported_apis.end());
}

bool kafka::api::dto::is_supported_version(api::registry::ApiKey key,
                                           uint16_t version) {
  auto api_details = get_api_details(std::to_underlying(key));
  if (!api_details) {
    return false;
  }
  return version >= api_details->min_version &&
         version <= api_details->max_version;
}
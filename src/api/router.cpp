#include "api/router.hpp"

kafka::KafkaRouter kafka::KafkaRouterBuilder::build() {
  auto termianl_node = [this](const api::dto::Request &request) {
    return this->controller_.handle(request);
  };
  KafkaRouter router{};
  HandlerFunc current_node = termianl_node;

  for (auto it = middlewares_.rbegin(); it != middlewares_.rend(); ++it) {
    auto current_mw = *it;
    auto next_node = current_node;

    current_node = [current_mw, next_node](
                       const api::dto::Request &req) -> api::dto::Response {
      return current_mw(req, next_node);
    };
  }

  router.compiled_pipeline_ = current_node;
  return router;
}

auto kafka::KafkaRouter::handle(const api::dto::Request &request)
    -> api::dto::Response {
  return compiled_pipeline_(request);
}

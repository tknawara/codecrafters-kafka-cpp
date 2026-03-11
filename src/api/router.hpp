#pragma once

#include "api/controller.hpp"
#include "api/dto/request.hpp"
#include "api/dto/response.hpp"
#include "api/types.hpp"

namespace kafka {
class KafkaRouterBuilder;
class KafkaRouter {
  friend class KafkaRouterBuilder;

public:
  auto handle(const api::dto::Request &request) -> api::dto::Response;

private:
  KafkaRouter() = default;
  HandlerFunc compiled_pipeline_;
};

class KafkaRouterBuilder {
public:
  KafkaRouterBuilder(KafkaController controller)
      : controller_(std::move(controller)) {}
  KafkaRouter build();
  KafkaRouterBuilder &use(MiddlewareFunc mw) {
    middlewares_.push_back(mw);
    return *this;
  }

private:
  std::vector<MiddlewareFunc> middlewares_;
  KafkaController controller_;
};
} // namespace kafka
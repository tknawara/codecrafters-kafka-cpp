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
  HandlerFunc compiled_pipeline_;
  HandlerFunc terminal_node_;
  KafkaController controller_;

  KafkaRouter(KafkaController controller) : controller_(controller) {
    terminal_node_ = [this](const api::dto::Request &request) {
      return this->controller_.handle(request);
    };
  }
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
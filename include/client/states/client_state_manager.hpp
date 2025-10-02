#pragma once

#include <memory>

#include "client_states.hpp"

// All calls to state transitions should be made through this manager
class client_state_manager {
 private:
  std::shared_ptr<generic_camera> camera;
  std::shared_ptr<robot_rpc_manager> rpc_manager;

  client_state_manager()
      : camera{std::make_shared<laptop_camera>()}, rpc_manager{std::make_shared<robot_rpc_manager>()} {
    bot::camera = camera;
    bot::rpc_manager = rpc_manager;
  }

  ~client_state_manager();

 public:
  static auto get_instance() -> client_state_manager& {
    static client_state_manager instance;
    return instance;
  }

  auto start() -> void { bot::start(); }
};

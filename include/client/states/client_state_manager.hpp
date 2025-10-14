#pragma once

#include <memory>

#include "client_states.hpp"

// Forward declarations
class generic_camera;
class robot_rpc_manager;

// All calls to state transitions should be made through this manager
class client_state_manager {
 private:
  std::shared_ptr<generic_camera> camera;
  std::shared_ptr<robot_rpc_manager> rpc_manager;

  client_state_manager();
  ~client_state_manager();

 public:
  static auto get_instance() -> client_state_manager& {
    static client_state_manager instance;
    return instance;
  }

  auto start() -> void { bot::start(); }
};

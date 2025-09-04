#pragma once

#include "client/client_states.hpp"
#include "client/zed_camera.hpp"

// All calls to state transitions should be made through this manager from one thread
class client_state_manager {
 private:
  zed_camera camera;

  client_state_manager();
  ~client_state_manager();

 public:
  static auto get_instance() -> client_state_manager& {
    static client_state_manager instance;
    return instance;
  }

  auto start() -> bool {
    if (!camera.start()) robot::dispatch(camera_error_event{});

    return true;
  }
};

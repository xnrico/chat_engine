#pragma once

#include "client/client_states.hpp"
#include "client/zed_camera.hpp"

// All calls to state transitions should be made through this manager
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
    camera.set_on_human_detected([]() { robot::dispatch(human_presence_event{true}); });
    camera.set_on_human_lost([]() { robot::dispatch(human_presence_event{false}); });
    robot::dispatch(init_success_event{});
    return true;
  }
};

#pragma once

#include <memory>

#include "client_states.hpp"

// All calls to state transitions should be made through this manager
class client_state_manager {
 private:
  std::shared_ptr<generic_camera> camera;
  std::shared_ptr<rtc_client> client;

  client_state_manager() : camera{std::make_shared<laptop_camera>()}, client{std::make_shared<rtc_client>()} {
    robot::camera = camera;
    robot::client = client;
    robot::start();
  }

  ~client_state_manager();

 public:
  static auto get_instance() -> client_state_manager& {
    static client_state_manager instance;
    return instance;
  }

  auto start() -> bool {
    if (!camera->start()) robot::dispatch(camera_error_event{});
    camera->set_on_human_detected([]() { robot::dispatch(human_presence_event{true}); });
    camera->set_on_human_lost([]() { robot::dispatch(human_presence_event{false}); });

    robot::dispatch(init_success_event{});
    return true;
  }
};

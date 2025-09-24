#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct init_state final : robot {
  auto entry() -> void override {
    LOG_INFO(logger, "[init::entry] Entering init state, performing initialization");
    if (!camera->start()) robot::dispatch(camera_error_event{});
    camera->set_on_human_detected([]() { robot::dispatch(human_presence_event{true}); });
    camera->set_on_human_lost([]() { robot::dispatch(human_presence_event{false}); });
    robot::dispatch(init_success_event{});
  }

  auto react(const init_success_event& e) -> void override {
    transit<idle_state>([&e]() -> void {
      // Action function
      LOG_INFO(logger, "[init::react] Initialization successful, transitioning to idle state");
    });
  }
  auto react(const camera_error_event& e) -> void override {
    transit<terminated_state>([&e]() -> void {
      // Action function
      LOG_ERROR(logger, "[init::react] Camera error occurred, transitioning to terminated state");
    });
  }

  auto get_state() const -> client_state override { return client_state::INIT; }
};
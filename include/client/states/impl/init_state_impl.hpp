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
    LOG_INFO(logger, "[{}::entry] Entering init state, performing initialization", to_string(get_state()));
    if (!camera->start()) robot::dispatch(camera_error_event{});
    camera->set_on_human_detected([]() { robot::dispatch(human_presence_event{true}); });
    camera->set_on_human_lost([]() { robot::dispatch(human_presence_event{false}); });
    client->set_on_stream_start([]() { robot::dispatch(server_ready_event{true}); });
    client->set_on_stream_failed([]() { robot::dispatch(server_ready_event{false}); });
    robot::dispatch(init_success_event{});
  }

  auto react(const init_success_event& e) -> void override {
    transit<idle_state>([this, &e]() -> void {
      // Action function
      LOG_INFO(logger, "[{}::react] Initialization successful, transitioning to idle state", to_string(get_state()));
    });
  }
  auto react(const camera_error_event& e) -> void override {
    transit<terminated_state>([this, &e]() -> void {
      // Action function
      LOG_ERROR(logger, "[{}::react] Camera error occurred, transitioning to terminated state", to_string(get_state()));
    });
  }

  auto get_state() const -> client_state override { return client_state::INIT; }
};
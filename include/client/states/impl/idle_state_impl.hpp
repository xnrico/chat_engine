#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct idle_state final : robot {
  auto react(const human_presence_event& e) -> void override {
    transit<wait_stream_camera_state>(
        [&e]() -> void {
          // Action function
          LOG_INFO(logger, "[idle::react] Human present, transitioning to active state");
        },
        [&e]() -> bool {
          // Condition function
          return e.present;
        });
  }

  auto get_state() const -> client_state override { return client_state::IDLE; }
};
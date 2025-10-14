#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct idle_state final : bot {
  auto react(const human_presence_event& e) -> void override {
    transit<recognition_state>([this]() -> void {
      // Action function
      LOG_DEBUG(logger, "[{}::react] Human present, transitioning to recognition state", to_string(get_state()));
    });
  }

  auto get_state() const -> client_state override { return client_state::IDLE; }
};
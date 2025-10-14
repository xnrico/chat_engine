#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct speech_state final : bot {
  auto react(const speech_success_event& e) -> void override {
    transit<response_state>([&e]() -> void {
      // Action function
      LOG_DEBUG(logger, "[speech::react] Speech successful, transitioning to response_state");
    });
  }

  auto react(const network_error_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      LOG_ERROR(logger, "[speech::react] Speech failed, transitioning to fault_state");
    });
  }

  auto react(const timeout_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      LOG_ERROR(logger, "[speech::react] Timeout occurred, transitioning to fault_state");
    });
  }

  auto get_state() const -> client_state override { return client_state::SPEECH; }
};
#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct greeting_state final : robot {
  auto react(const greeting_success_event& e) -> void override {
    transit<detect_speech_state>([&e]() -> void {
      // Action function
      LOG_INFO(logger, "[greeting::react] Greeting successful, transitioning to detect_speech_state");
    });
  }

  auto react(const greeting_failure_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      LOG_ERROR(logger, "[greeting::react] Greeting failed, transitioning to fault_state");
    });
  }

  auto react(const timeout_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      LOG_ERROR(logger, "[greeting::react] Timeout occurred, transitioning to fault_state");
    });
  }

  auto react(const playback_error_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      LOG_ERROR(logger, "[greeting::react] Playback error occurred, transitioning to fault_state");
    });
  }

  auto get_state() const -> client_state override { return client_state::GREETING; }
};
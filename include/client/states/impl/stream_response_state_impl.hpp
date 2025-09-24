#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct stream_response_state final : robot {
  auto react(const stream_response_success_event& e) -> void override {
    transit<detect_speech_state>([&e]() -> void {
      // Action function
      LOG_INFO(logger, "[stream_response::react] Stream response successful, transitioning to detect_speech_state");
    });
  }

  auto react(const stream_response_failure_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      LOG_ERROR(logger, "[stream_response::react] Stream response failed, transitioning to fault_state");
    });
  }

  auto react(const timeout_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      LOG_ERROR(logger, "[stream_response::react] Timeout occurred, transitioning to fault_state");
    });
  }

  auto react(const playback_error_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      LOG_ERROR(logger, "[stream_response::react] Playback error occurred, transitioning to fault_state");
    });
  }

  auto get_state() const -> client_state override { return client_state::STREAM_RESPONSE; }
};

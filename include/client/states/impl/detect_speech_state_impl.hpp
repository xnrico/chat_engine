#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct detect_speech_state final : robot {
  auto react(const user_speech_detected_event& e) -> void override {
    transit<wait_stream_speech_state>(
        [&e]() -> void {
          // Action function
          LOG_INFO(logger, "[detect_speech::react] Speech detected, transitioning to wait_stream_speech_state");
        },
        [&e]() -> bool { return e.detected; });
  }

  auto react(const timeout_event& e) -> void override {
    transit<idle_state>([&e]() -> void {
      // Action function
      LOG_INFO(logger, "[detect_speech::react] Timeout occurred, transitioning to idle_state");
    });
  }

  auto get_state() const -> client_state override { return client_state::DETECT_SPEECH; }
};
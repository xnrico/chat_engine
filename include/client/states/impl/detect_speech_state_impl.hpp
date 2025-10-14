#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct detect_speech_state final : bot {
  auto react(const vad_success_event& e) -> void override {
    transit<speech_state>([&e]() -> void {
      // Action function
      LOG_INFO(logger, "[detect_speech::react] Speech detected, transitioning to speech_state");
    });
  }

  auto react(const timeout_event& e) -> void override {
    transit<idle_state>([&e]() -> void {
      // Action function
      LOG_INFO(logger, "[detect_speech::react] Timeout occurred, transitioning to idle_state");
    });
  }

  auto get_state() const -> client_state override { return client_state::DETECT_SPEECH; }
};
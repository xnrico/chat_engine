#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct stream_speech_state final : robot {
  auto react(const stream_speech_success_event& e) -> void override {
    transit<wait_stream_response_state>([&e]() -> void {
      // Action function
      LOG_INFO(logger, "[stream_speech::react] Speech stream successful, transitioning to wait_stream_response_state");
    });
  }

  auto react(const stream_speech_failure_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      LOG_ERROR(logger, "[stream_speech::react] Speech stream failed, transitioning to fault_state");
    });
  }

  auto react(const timeout_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      LOG_ERROR(logger, "[stream_speech::react] Timeout occurred, transitioning to fault_state");
    });
  }

  auto get_state() const -> client_state override { return client_state::STREAM_SPEECH; }
};
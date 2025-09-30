#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct wait_stream_response_state final : bot {
  auto react(const server_ready_event& e) -> void override {
    transit<stream_response_state>(
        [&e]() -> void {
          // Action function
          LOG_INFO(logger, "[wait_stream_response::react] Server ready for response stream, transitioning to stream_response_state");
        },
        [&e]() -> bool {
          // Condition function
          return e.ready;
        });

    transit<fault_state>(
        [&e]() -> void {
          // Action function
          LOG_ERROR(logger, "[wait_stream_response::react] Fault occurred, transitioning to fault_state");
        },
        [&e]() -> bool {
          // Condition function
          return !e.ready;
        });
  }

  auto react(const timeout_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      LOG_ERROR(logger, "[wait_stream_response::react] Timeout occurred, transitioning to fault_state");
    });
  }

  auto get_state() const -> client_state override { return client_state::WAIT_STREAM_RESPONSE; }
};
#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct wait_stream_camera_state final : bot {
 private:
  auto process() -> void {}

 public:
  auto entry() -> void override { rpc_manager->init_camera_stream(); }

  auto react(const server_ready_event& e) -> void override {
    transit<stream_camera_state>(
        [this, &e]() -> void {
          // Action function
          LOG_INFO(logger, "[{}::react] Server ready for camera stream, transitioning to stream_camera_state",
                   to_string(get_state()));
        },
        [&e]() -> bool {
          // Condition function
          return e.ready;
        });

    transit<idle_state>(
        [this, &e]() -> void {
          // Action function
          LOG_ERROR(logger, "[{}::react] Server not ready for camera stream, transitioning to idle_state",
                    to_string(get_state()));
        },
        [&e]() -> bool {
          // Condition function
          return !e.ready;
        });
  }

  auto react(const timeout_event& e) -> void override { transit<fault_state>(); }
  auto get_state() const -> client_state override { return client_state::WAIT_STREAM_CAMERA; }
};
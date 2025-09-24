#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct wait_stream_camera_state final : robot {
 private:
  std::thread work_thread;

  auto process() -> void {}

 public:
  auto entry() -> void override {
    work_thread = std::thread([this]() { this->process(); });
  }

  auto exit() -> void override {
    if (work_thread.joinable()) {
      work_thread.join();
    }
  }

  auto react(const server_ready_event& e) -> void override {
    transit<stream_camera_state>(
        [&e]() -> void {
          // Action function
          LOG_INFO(logger, "[wait_stream_camera::react] Server ready for camera stream, transitioning to stream_camera_state");
        },
        [&e]() -> bool {
          // Condition function
          return e.ready;
        });
  }

  auto react(const timeout_event& e) -> void override { transit<fault_state>(); }
  auto get_state() const -> client_state override { return client_state::WAIT_STREAM_CAMERA; }
};
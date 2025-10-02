#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct stream_camera_state final : bot {
  auto react(const facial_recognition_response_event& e) -> void override {
    transit<greeting_state>(
        [&e]() -> void {
          // Action function
          LOG_INFO(logger, "[stream_camera::react] Facial recognition indicates not greeted, transitioning to greeting_state");
        },
        [&e]() -> bool {
          // Condition function
          return !e.greeted;
        });

    transit<wait_stream_speech_state>(
        [&e]() -> void {
          // Action function
          LOG_INFO(logger, "[stream_camera::react] Facial recognition indicates greeted, transitioning to wait_stream_speech_state");
        },
        [&e]() -> bool {
          // Condition function
          return e.greeted;
        });
  }

  auto react(const timeout_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      LOG_ERROR(logger, "[stream_camera::react] Timeout occurred, transitioning to fault_state");
    });
  }

  auto react(const camera_error_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      LOG_ERROR(logger, "[stream_camera::react] Camera error occurred, transitioning to fault_state");
    });
  }

  auto get_state() const -> client_state override { return client_state::STREAM_CAMERA; }
};
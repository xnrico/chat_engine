#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "tinyfsm/tinyfsm.hpp"

using namespace std::chrono_literals;

// Forward declarations for state machine and states
struct robot;

//=============================================================================
// STATE DECLARATIONS
//=============================================================================

/**
 * @brief Initial idle state - monitors for object detection
 * 
 * Tasks:
 * - Poll object detection status at constant frequency
 * 
 * Transitions:
 * - object detected (true) → wait_stream_camera_state
 */
struct idle_state;

/**
 * @brief Prepares server for camera streaming
 * 
 * Tasks:
 * - RPC call to server to confirm readiness for camera stream
 * 
 * Transitions:
 * - server ready for stream → stream_camera_state
 * - server not ready for stream → idle_state
 * - timeout (no response) → fault_state
 */
struct wait_stream_camera_state;

/**
 * @brief Handles video streaming and facial recognition
 * 
 * Tasks:
 * - Initiate video streaming via WebRTC to server
 * - Receive response from server to determine if greeting is needed
 * 
 * Transitions:
 * - server sends "not greeted" response → greeting_state
 * - server sends "greeted" response → waiting_for_speech_state
 * - timeout (no response) → fault_state
 */
struct stream_camera_state;

/**
 * @brief Plays greeting audio from server
 * 
 * Tasks:
 * - Accept audio stream via WebRTC from server
 * - Playback response audio through speaker
 * 
 * Transitions:
 * - audio stream finished successfully → detect_speech_state
 * - audio stream failed → fault_state
 * - timeout (no stream from server) → fault_state
 * - playback error → fault_state
 */
struct greeting_state;

/**
 * @brief Monitors for user voice activity
 * 
 * Tasks:
 * - Poll VAD (Voice Activity Detection) and wait for user speech
 * 
 * Transitions:
 * - user voice activity detected → wait_stream_speech_state
 * - timeout (no voice activity) → idle_state
 */
struct detect_speech_state;

/**
 * @brief Prepares server for audio streaming
 * 
 * Tasks:
 * - RPC call to server to confirm readiness for audio stream
 * 
 * Transitions:
 * - server ready for audio stream → stream_speech_state
 * - server not ready for audio stream → idle_state
 * - timeout (no response) → fault_state
 */
struct wait_stream_speech_state;

/**
 * @brief Streams user speech to server
 * 
 * Tasks:
 * - Initiate audio streaming via WebRTC to server
 * - Receive response from server confirming stream reception
 * 
 * Transitions:
 * - server sends normal status → wait_stream_response_state
 * - server sends bad response → fault_state
 * - timeout (no response) → fault_state
 */
struct stream_speech_state;

/**
 * @brief Waits for server to prepare response audio
 * 
 * Tasks:
 * - RPC call to server to confirm readiness to stream response audio
 * 
 * Transitions:
 * - server ready for audio stream → stream_response_state
 * - timeout (no response) → fault_state
 */
struct wait_stream_response_state;

/**
 * @brief Plays response audio from server
 * 
 * Tasks:
 * - Accept WebRTC audio stream from server
 * - Play back audio through speaker
 * 
 * Transitions:
 * - audio stream finished successfully → detect_speech_state
 * - audio stream failed → fault_state
 * - timeout (no stream from server) → fault_state
 * - playback error → fault_state
 */
struct stream_response_state;

/**
 * @brief Error handling state - logs faults and recovers
 * 
 * Tasks:
 * - Log fault information for debugging
 * - Perform cleanup operations
 * 
 * Transitions:
 * - Always → idle_state (automatic recovery)
 */
struct fault_state;

//=============================================================================
// EVENT DECLARATIONS
//=============================================================================

// Represents a change in human presence status
struct human_presence_event : tinyfsm::Event {
  std::string name = "human_presence_event";
  bool present;
  human_presence_event(bool p) : present(p) {}
};

struct user_speech_detected_event : tinyfsm::Event {
  std::string name = "user_speech_detected_event";
  bool detected;
  user_speech_detected_event(bool d) : detected(d) {}
};

// Events for state transitions
struct generic_event : tinyfsm::Event {
  std::string name = "generic_event";
};

struct facial_recognition_response_event : tinyfsm::Event {
  std::string name = "facial_recognition_response_event";
};
struct webrtc_video_stopped_event : tinyfsm::Event {
  std::string name = "webrtc_video_stopped_event";
};
struct webrtc_greeting_audio_finished_event : tinyfsm::Event {
  std::string name = "webrtc_greeting_audio_finished_event";
};
struct webrtc_response_audio_finished_event : tinyfsm::Event {
  std::string name = "webrtc_response_audio_finished_event";
};
struct timeout_event : tinyfsm::Event {
  std::string name = "timeout_event";
};
struct network_error_event : tinyfsm::Event {
  std::string name = "network_error_event";
};

// Main state machine for robot (chat client)
struct robot : public tinyfsm::MealyMachine<robot> {
 public:
  virtual void react(const generic_event& e) {
    // Default event handler
    (void)e;
    std::cout << "Robot cannot handle event" << std::endl;
  }

  virtual void react(const human_presence_event& e) {
    std::cout << "\033[31m" << "[" << this->get_state() << "::react] cannot handle event [human_presence_event]\n" << "\033[0m";
  }

  virtual void react(const user_speech_detected_event&) {
    std::cout << "\033[31m" << "[" << this->get_state() << "::react] cannot handle event [user_speech_detected_event]\n" << "\033[0m";
  }
  virtual void react(const facial_recognition_response_event&) {
    std::cout << "\033[31m" << "[" << this->get_state() << "::react] cannot handle event [facial_recognition_response_event]\n" << "\033[0m";
  }
  virtual void react(const webrtc_video_stopped_event&) {
    std::cout << "\033[31m" << "[" << this->get_state() << "::react] cannot handle event [webrtc_video_stopped_event]\n" << "\033[0m";
  }
  virtual void react(const webrtc_greeting_audio_finished_event&) {
    std::cout << "\033[31m" << "[" << this->get_state() << "::react] cannot handle event [webrtc_greeting_audio_finished_event]\n" << "\033[0m";
  }
  virtual void react(const webrtc_response_audio_finished_event&) {
    std::cout << "\033[31m" << "[" << this->get_state() << "::react] cannot handle event [webrtc_response_audio_finished_event]\n" << "\033[0m";
  }
  virtual void react(const timeout_event&) {
    std::cout << "\033[31m" << "[" << this->get_state() << "::react] cannot handle event [timeout_event]\n" << "\033[0m";
  }
  virtual void react(const network_error_event&) {
    std::cout << "\033[31m" << "[" << this->get_state() << "::react] cannot handle event [network_error_event]\n" << "\033[0m";
  }

  virtual void entry() {}
  virtual void exit() {}
  virtual std::string get_state() const = 0;
};

// States
struct idle_state final : robot {
  void react(const generic_event& e) override {
    std::cout << "[idle::react] Idle state ignoring generic event" << std::endl;
  }

  void react(const human_presence_event& e) override {
    transit<wait_stream_camera_state>(
        [&e]() -> void {
          // Action function
          std::cout << "[idle::react] Human present, transitioning to active state" << std::endl;
        },
        [&e]() -> bool {
          // Condition function
          return e.present;
        });
  }

  std::string get_state() const override { return "idle"; }
};

FSM_INITIAL_STATE(robot, idle_state);
#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "client/client_common.hpp"
#include "tinyfsm/tinyfsm.hpp"

using namespace std::chrono_literals;

enum struct client_state {
  INIT,
  TERMINATED,
  IDLE,
  WAIT_STREAM_CAMERA,
  STREAM_CAMERA,
  GREETING,
  WAITING_FOR_SPEECH,
  DETECT_SPEECH,
  WAIT_STREAM_SPEECH,
  STREAM_SPEECH,
  WAIT_STREAM_RESPONSE,
  STREAM_RESPONSE,
  FAULT
};

inline auto to_string(client_state state) -> std::string {
  switch (state) {
    case client_state::INIT:
      return "INIT";
    case client_state::TERMINATED:
      return "TERMINATED";
    case client_state::IDLE:
      return "IDLE";
    case client_state::WAIT_STREAM_CAMERA:
      return "WAIT_STREAM_CAMERA";
    case client_state::STREAM_CAMERA:
      return "STREAM_CAMERA";
    case client_state::GREETING:
      return "GREETING";
    case client_state::WAITING_FOR_SPEECH:
      return "WAITING_FOR_SPEECH";
    case client_state::DETECT_SPEECH:
      return "DETECT_SPEECH";
    case client_state::WAIT_STREAM_SPEECH:
      return "WAIT_STREAM_SPEECH";
    case client_state::STREAM_SPEECH:
      return "STREAM_SPEECH";
    case client_state::WAIT_STREAM_RESPONSE:
      return "WAIT_STREAM_RESPONSE";
    case client_state::STREAM_RESPONSE:
      return "STREAM_RESPONSE";
    case client_state::FAULT:
      return "FAULT";
    default:
      return "UNKNOWN";
  }
}

// Forward declarations for state machine and states
struct robot;

//=============================================================================
// STATE DECLARATIONS
//=============================================================================

/**
 * @brief Initial state
 *
 * Tasks:
 * - Starts all services and check for error
 *
 * Transitions:
 * - No errors → idle_state
 * - Errors → terminated_state
 */

struct init_state;

/**
 * @brief Terminated state
 *
 * Tasks:
 * - Stops all services and performs cleanup
 *
 * Transitions:
 * - No transitions
 */

struct terminated_state;

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

/* Non-state specific events*/
struct generic_event;
struct timeout_event;
struct network_error_event;
struct playback_error_event;
struct server_ready_event;  // binary

/* Initial state events*/
struct init_success_event;
struct camera_error_event;

/* Idle state events*/
struct human_presence_event;  // binary

/* Stream events*/
struct facial_recognition_response_event;  // binary

/* Greeting events*/
struct greeting_success_event;
struct greeting_failure_event;

/* Detect speech events*/
struct user_speech_detected_event;  // binary

/* Stream speech events*/
struct stream_speech_success_event;
struct stream_speech_failure_event;

/* Stream response events*/
struct stream_response_success_event;
struct stream_response_failure_event;

//=============================================================================
// STATE MACHINE DEFINITIONS
//=============================================================================

// Main state machine for robot (chat client)
struct robot : public tinyfsm::MealyMachine<robot> {
  static std::shared_ptr<generic_camera> camera;
  static std::shared_ptr<rtc_client> client;

  virtual void react(const generic_event&) {
    // Default event handler
    std::cout << "\033[31m" << "[" << to_string(this->get_state()) << "::react] cannot handle event [generic_event]\n"
              << "\033[0m";
  }

  virtual void react(const timeout_event&) {
    std::cout << "\033[31m" << "[" << to_string(this->get_state()) << "::react] cannot handle event [timeout_event]\n"
              << "\033[0m";
  }

  virtual void react(const network_error_event&) {
    std::cout << "\033[31m" << "[" << to_string(this->get_state())
              << "::react] cannot handle event [network_error_event]\n"
              << "\033[0m";
  }

  virtual void react(const playback_error_event&) {
    std::cout << "\033[31m" << "[" << to_string(this->get_state())
              << "::react] cannot handle event [playback_error_event]\n"
              << "\033[0m";
  }

  virtual void react(const server_ready_event&) {
    std::cout << "\033[31m" << "[" << to_string(this->get_state())
              << "::react] cannot handle event [server_ready_event]\n"
              << "\033[0m";
  }

  virtual void react(const init_success_event&) {
    std::cout << "\033[31m" << "[" << to_string(this->get_state())
              << "::react] cannot handle event [init_success_event]\n"
              << "\033[0m";
  }

  virtual void react(const camera_error_event&) {
    std::cout << "\033[31m" << "[" << to_string(this->get_state())
              << "::react] cannot handle event [camera_error_event]\n"
              << "\033[0m";
  }

  virtual void react(const human_presence_event&) {
    std::cout << "\033[31m" << "[" << to_string(this->get_state())
              << "::react] cannot handle event [human_presence_event]\n"
              << "\033[0m";
  }

  virtual void react(const facial_recognition_response_event&) {
    std::cout << "\033[31m" << "[" << to_string(this->get_state())
              << "::react] cannot handle event [facial_recognition_response_event]\n"
              << "\033[0m";
  }

  virtual void react(const greeting_success_event&) {
    std::cout << "\033[31m" << "[" << to_string(this->get_state())
              << "::react] cannot handle event [greeting_success_event]\n"
              << "\033[0m";
  }

  virtual void react(const greeting_failure_event&) {
    std::cout << "\033[31m" << "[" << to_string(this->get_state())
              << "::react] cannot handle event [greeting_failure_event]\n"
              << "\033[0m";
  }

  virtual void react(const user_speech_detected_event&) {
    std::cout << "\033[31m" << "[" << to_string(this->get_state())
              << "::react] cannot handle event [user_speech_detected_event]\n"
              << "\033[0m";
  }

  virtual void react(const stream_speech_success_event&) {
    std::cout << "\033[31m" << "[" << to_string(this->get_state())
              << "::react] cannot handle event [stream_speech_success_event]\n"
              << "\033[0m";
  }

  virtual void react(const stream_speech_failure_event&) {
    std::cout << "\033[31m" << "[" << to_string(this->get_state())
              << "::react] cannot handle event [stream_speech_failure_event]\n"
              << "\033[0m";
  }

  virtual void react(const stream_response_success_event&) {
    std::cout << "\033[31m" << "[" << to_string(this->get_state())
              << "::react] cannot handle event [stream_response_success_event]\n"
              << "\033[0m";
  }

  virtual void react(const stream_response_failure_event&) {
    std::cout << "\033[31m" << "[" << to_string(this->get_state())
              << "::react] cannot handle event [stream_response_failure_event]\n"
              << "\033[0m";
  }

  virtual void entry() {}
  virtual void exit() {}
  virtual client_state get_state() const = 0;
};

//=============================================================================
// EVENT DEFINITIONS
//=============================================================================

/* Non-state specific events*/
struct generic_event : tinyfsm::Event {
  std::string name = "generic_event";
};

struct timeout_event : tinyfsm::Event {
  std::string name = "timeout_event";
};

struct network_error_event : tinyfsm::Event {
  std::string name = "network_error_event";
};

struct playback_error_event : tinyfsm::Event {
  std::string name = "playback_error_event";
};

struct server_ready_event : tinyfsm::Event {
  std::string name = "server_ready_event";
  bool ready;
  server_ready_event(bool r) : ready{r} {}
  server_ready_event() : ready{true} {}
};

/* Initial state events*/
struct init_success_event : tinyfsm::Event {
  std::string name = "init_success_event";
};

struct camera_error_event : tinyfsm::Event {
  std::string name = "camera_error_event";
};

/* Idle state events*/
struct human_presence_event : tinyfsm::Event {
  std::string name = "human_presence_event";
  bool present;
  human_presence_event(bool p) : present{p} {}
  human_presence_event() : present{true} {}
};

/* Stream events*/
struct facial_recognition_response_event : tinyfsm::Event {
  std::string name = "facial_recognition_response_event";
  bool greeted;  // Indicates if the user was greeted
  facial_recognition_response_event(bool g) : greeted{g} {}
  facial_recognition_response_event() : greeted{false} {}
};

/* Greeting events*/
struct greeting_success_event : tinyfsm::Event {
  std::string name = "greeting_success_event";
};

struct greeting_failure_event : tinyfsm::Event {
  std::string name = "greeting_failure_event";
};

/* Detect speech events*/
struct user_speech_detected_event : tinyfsm::Event {
  std::string name = "user_speech_detected_event";
  bool detected;
  user_speech_detected_event(bool d) : detected{d} {}
  user_speech_detected_event() : detected{true} {}
};

/* Stream speech events*/
struct stream_speech_success_event : tinyfsm::Event {
  std::string name = "stream_speech_success_event";
};

struct stream_speech_failure_event : tinyfsm::Event {
  std::string name = "stream_speech_failure_event";
};

/* Stream response events*/
struct stream_response_success_event : tinyfsm::Event {
  std::string name = "stream_response_success_event";
};

struct stream_response_failure_event : tinyfsm::Event {
  std::string name = "stream_response_failure_event";
};

//=============================================================================
// STATE DECLARATIONS
//=============================================================================
struct init_state final : robot {
  auto react(const init_success_event& e) -> void override {
    transit<idle_state>([&e]() -> void {
      // Action function
      std::cout << "[init::react] Initialization successful, transitioning to idle state" << std::endl;
    });
  }
  auto react(const camera_error_event& e) -> void override {
    transit<terminated_state>([&e]() -> void {
      // Action function
      std::cout << "[init::react] Camera error occurred, transitioning to terminated state" << std::endl;
    });
  }

  auto get_state() const -> client_state override { return client_state::INIT; }
};

struct terminated_state final : robot {
  auto get_state() const -> client_state override { return client_state::TERMINATED; }
  auto entry() -> void override { std::cout << "[terminated::entry] Entering terminated state" << std::endl; }
};

struct idle_state final : robot {
  auto react(const human_presence_event& e) -> void override {
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

  auto get_state() const -> client_state override { return client_state::IDLE; }
};

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
          std::cout
              << "[wait_stream_camera::react] Server ready for camera stream, transitioning to stream_camera_state"
              << std::endl;
        },
        [&e]() -> bool {
          // Condition function
          return e.ready;
        });
  }

  auto react(const timeout_event& e) -> void override { transit<fault_state>(); }
  auto get_state() const -> client_state override { return client_state::WAIT_STREAM_CAMERA; }
};

struct stream_camera_state final : robot {
  auto react(const facial_recognition_response_event& e) -> void override {
    transit<greeting_state>(
        [&e]() -> void {
          // Action function
          std::cout
              << "[stream_camera::react] Facial recognition indicates not greeted, transitioning to greeting_state"
              << std::endl;
        },
        [&e]() -> bool {
          // Condition function
          return !e.greeted;
        });

    transit<wait_stream_speech_state>(
        [&e]() -> void {
          // Action function
          std::cout << "[stream_camera::react] Facial recognition indicates greeted, transitioning to "
                       "wait_stream_speech_state"
                    << std::endl;
        },
        [&e]() -> bool {
          // Condition function
          return e.greeted;
        });
  }

  auto react(const timeout_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      std::cout << "[stream_camera::react] Timeout occurred, transitioning to fault_state" << std::endl;
    });
  }

  auto get_state() const -> client_state override { return client_state::STREAM_CAMERA; }
};

struct greeting_state final : robot {
  auto react(const greeting_success_event& e) -> void override {
    transit<detect_speech_state>([&e]() -> void {
      // Action function
      std::cout << "[greeting::react] Greeting successful, transitioning to detect_speech_state" << std::endl;
    });
  }

  auto react(const greeting_failure_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      std::cout << "[greeting::react] Greeting failed, transitioning to fault_state" << std::endl;
    });
  }

  auto react(const timeout_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      std::cout << "[greeting::react] Timeout occurred, transitioning to fault_state" << std::endl;
    });
  }

  auto react(const playback_error_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      std::cout << "[greeting::react] Playback error occurred, transitioning to fault_state" << std::endl;
    });
  }

  auto get_state() const -> client_state override { return client_state::GREETING; }
};

struct detect_speech_state final : robot {
  auto react(const user_speech_detected_event& e) -> void override {
    transit<wait_stream_speech_state>(
        [&e]() -> void {
          // Action function
          std::cout << "[detect_speech::react] Speech detected, transitioning to wait_stream_speech_state" << std::endl;
        },
        [&e]() -> bool { return e.detected; });
  }

  auto react(const timeout_event& e) -> void override {
    transit<idle_state>([&e]() -> void {
      // Action function
      std::cout << "[detect_speech::react] Timeout occurred, transitioning to idle_state" << std::endl;
    });
  }

  auto get_state() const -> client_state override { return client_state::DETECT_SPEECH; }
};

struct wait_stream_speech_state final : robot {
  auto react(const server_ready_event& e) -> void override {
    transit<stream_speech_state>(
        [&e]() -> void {
          // Action function
          std::cout
              << "[wait_stream_speech::react] Server ready for speech stream, transitioning to stream_speech_state"
              << std::endl;
        },
        [&e]() -> bool {
          // Condition function
          return e.ready;
        });

    transit<fault_state>(
        [&e]() -> void {
          // Action function
          std::cout << "[wait_stream_speech::react] Fault occurred, transitioning to fault_state" << std::endl;
        },
        [&e]() -> bool {
          // Condition function
          return !e.ready;
        });
  }

  auto react(const timeout_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      std::cout << "[wait_stream_speech::react] Timeout occurred, transitioning to fault_state" << std::endl;
    });
  }

  auto get_state() const -> client_state override { return client_state::WAIT_STREAM_SPEECH; }
};

struct stream_speech_state final : robot {
  auto react(const stream_speech_success_event& e) -> void override {
    transit<wait_stream_response_state>([&e]() -> void {
      // Action function
      std::cout << "[stream_speech::react] Speech stream successful, transitioning to wait_stream_response_state"
                << std::endl;
    });
  }

  auto react(const stream_speech_failure_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      std::cout << "[stream_speech::react] Speech stream failed, transitioning to fault_state" << std::endl;
    });
  }

  auto react(const timeout_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      std::cout << "[stream_speech::react] Timeout occurred, transitioning to fault_state" << std::endl;
    });
  }

  auto get_state() const -> client_state override { return client_state::STREAM_SPEECH; }
};

struct wait_stream_response_state final : robot {
  auto react(const server_ready_event& e) -> void override {
    transit<stream_response_state>(
        [&e]() -> void {
          // Action function
          std::cout << "[wait_stream_response::react] Server ready for response stream, transitioning to "
                       "stream_response_state"
                    << std::endl;
        },
        [&e]() -> bool {
          // Condition function
          return e.ready;
        });

    transit<fault_state>(
        [&e]() -> void {
          // Action function
          std::cout << "[wait_stream_response::react] Fault occurred, transitioning to fault_state" << std::endl;
        },
        [&e]() -> bool {
          // Condition function
          return !e.ready;
        });
  }

  auto react(const timeout_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      std::cout << "[wait_stream_response::react] Timeout occurred, transitioning to fault_state" << std::endl;
    });
  }

  auto get_state() const -> client_state override { return client_state::WAIT_STREAM_RESPONSE; }
};

struct stream_response_state final : robot {
  auto react(const stream_response_success_event& e) -> void override {
    transit<detect_speech_state>([&e]() -> void {
      // Action function
      std::cout << "[stream_response::react] Stream response successful, transitioning to detect_speech_state"
                << std::endl;
    });
  }

  auto react(const stream_response_failure_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      std::cout << "[stream_response::react] Stream response failed, transitioning to fault_state" << std::endl;
    });
  }

  auto react(const timeout_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      std::cout << "[stream_response::react] Timeout occurred, transitioning to fault_state" << std::endl;
    });
  }

  auto react(const playback_error_event& e) -> void override {
    transit<fault_state>([&e]() -> void {
      // Action function
      std::cout << "[stream_response::react] Playback error occurred, transitioning to fault_state" << std::endl;
    });
  }

  auto get_state() const -> client_state override { return client_state::STREAM_RESPONSE; }
};

struct fault_state final : robot {
  auto entry() -> void override {
    std::cout << "[fault::entry] Entering fault state, performing cleanup and logging" << std::endl;
    // Perform cleanup and logging here
    // After handling the fault, automatically transition back to idle state
    transit<idle_state>([]() -> void { std::cout << "[fault::entry] Recovering to idle state" << std::endl; });
  }

  auto get_state() const -> client_state override { return client_state::FAULT; }
};

//=============================================================================
// INITIAL STATE - defined in client_states.cpp
//=============================================================================
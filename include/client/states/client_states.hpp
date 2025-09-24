#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

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
struct reset_event;
struct terminate_event;

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
// STATE MACHINE DECLARATIONS
//=============================================================================

struct robot : public tinyfsm::MealyMachine<robot> {
  static std::shared_ptr<generic_camera> camera;
  static std::shared_ptr<rtc_client> client;
  static std::shared_ptr<quill::Logger> logger;

  virtual void react(const reset_event&);
  virtual void react(const terminate_event&);
  virtual void react(const generic_event&);
  virtual void react(const timeout_event&);
  virtual void react(const network_error_event&);
  virtual void react(const playback_error_event&);
  virtual void react(const server_ready_event&);
  virtual void react(const init_success_event&);
  virtual void react(const camera_error_event&);
  virtual void react(const human_presence_event&);
  virtual void react(const facial_recognition_response_event&);
  virtual void react(const greeting_success_event&);
  virtual void react(const greeting_failure_event&);
  virtual void react(const user_speech_detected_event&);
  virtual void react(const stream_speech_success_event&);
  virtual void react(const stream_speech_failure_event&);
  virtual void react(const stream_response_success_event&);
  virtual void react(const stream_response_failure_event&);

  virtual void entry();
  virtual void exit();
  virtual client_state get_state() const = 0;
};

//=============================================================================
// STATE MACHINE DEFINITIONS
//=============================================================================

#include "impl/fsm_impl.hpp"

//=============================================================================
// EVENT DEFINITIONS
//=============================================================================

#include "impl/event_impl.hpp"

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

#include "impl/detect_speech_state_impl.hpp"
#include "impl/fault_state_impl.hpp"
#include "impl/greeting_state_impl.hpp"
#include "impl/idle_state_impl.hpp"
#include "impl/init_state_impl.hpp"
#include "impl/stream_camera_state_impl.hpp"
#include "impl/stream_response_state_impl.hpp"
#include "impl/stream_speech_state_impl.hpp"
#include "impl/terminated_state_impl.hpp"
#include "impl/wait_stream_camera_state_impl.hpp"
#include "impl/wait_stream_response_impl.hpp"
#include "impl/wait_stream_speech_state_impl.hpp"

//=============================================================================
// INITIAL STATE - defined in client_states.cpp
//=============================================================================
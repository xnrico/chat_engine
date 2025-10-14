#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "tinyfsm/tinyfsm.hpp"

using namespace std::chrono_literals;

enum struct client_state { INIT, TERMINATED, IDLE, RECOGNITION, GREETING, DETECT_SPEECH, SPEECH, RESPONSE, FAULT };

inline auto to_string(client_state state) -> std::string {
  switch (state) {
    case client_state::INIT:
      return "INIT";
    case client_state::TERMINATED:
      return "TERMINATED";
    case client_state::IDLE:
      return "IDLE";
    case client_state::RECOGNITION:
      return "RECOGNITION";
    case client_state::GREETING:
      return "GREETING";
    case client_state::DETECT_SPEECH:
      return "DETECT_SPEECH";
    case client_state::SPEECH:
      return "SPEECH";
    case client_state::RESPONSE:
      return "RESPONSE";
    case client_state::FAULT:
      return "FAULT";
    default:
      return "UNKNOWN";
  }
}

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
 * - object detected (true) → recognition_state
 * - object lost (false) → idle_state (no state change)
 */
struct idle_state;

/**
 * @brief Acquires facial recognition result from server
 *
 * Tasks:
 * - RPC call to FR server (Python) to get recognition result
 *
 * Transitions:
 * - normal response → greeting_state
 * - bad response (e.g., no face detected) → fault_state
 * - timeout (no response) → fault_state
 */
struct recognition_state;

/**
 * @brief Request and play greeting audio sent from server
 *
 * Tasks:
 * - Request greeting audio data from server
 *
 * Transitions:
 * - audio data received successfully and playback successful → detect_speech_state
 * - bad response (e.g., no audio) → fault_state
 * - timeout (no data from server) → fault_state
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
 * - user voice activity detected → speech_state
 * - timeout (no voice activity) → idle_state
 */
struct detect_speech_state;

/**
 * @brief Send user speech to server
 *
 * Tasks:
 * - RPC call to server to confirm reception of user speech
 *
 * Transitions:
 * - server received speech → response_state
 * - bad response (e.g., server error) → fault_state
 * - timeout (no response) → fault_state
 */
struct speech_state;

/**
 * @brief Streams server response audio to client
 *
 * Tasks:
 * - Receive response from server and play audio through speaker
 *
 * Transitions:
 * - audio data received successfully and playback successful → detect_speech_state
 * - bad response (e.g., no audio) → fault_state
 * - timeout (no data from server) → fault_state
 * - playback error → fault_state
 */
struct response_state;

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
struct reset_event;
struct terminate_event;

/* Initial state events*/
struct init_success_event;
struct camera_error_event;

/* Idle state events*/
struct human_presence_event;

/* Stream events*/
struct fr_success_event;

/* Greeting events*/
struct greeting_success_event;

/* Detect speech events*/
struct vad_success_event;

/* Speech events*/
struct speech_success_event;

/* Response events*/
struct response_success_event;

//=============================================================================
// STATE MACHINE DECLARATIONS
//=============================================================================

struct bot;

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
#include "impl/recognition_state_impl.hpp"
#include "impl/response_state_impl.hpp"
#include "impl/speech_state_impl.hpp"
#include "impl/terminated_state_impl.hpp"

//=============================================================================
// INITIAL STATE - defined in client_states.cpp
//=============================================================================
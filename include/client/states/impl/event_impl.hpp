#pragma once

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

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

struct reset_event : tinyfsm::Event {
  std::string name = "reset_event";
};

struct terminated_event : tinyfsm::Event {
  std::string name = "terminated_event";
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
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
};

/* Stream events*/
struct fr_success_event : tinyfsm::Event {
  std::string name = "fr_success_event";
};

/* Greeting events*/
struct greeting_success_event : tinyfsm::Event {
  std::string name = "greeting_success_event";
};

/* Detect speech events*/
struct vad_success_event : tinyfsm::Event {
  std::string name = "vad_success_event";
};

/* Stream speech events*/
struct speech_success_event : tinyfsm::Event {
  std::string name = "speech_success_event";
};

/* Stream response events*/
struct response_success_event : tinyfsm::Event {
  std::string name = "response_success_event";
};

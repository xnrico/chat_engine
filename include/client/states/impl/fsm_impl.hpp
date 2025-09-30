#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE MACHINE DECLARATIONS
//=============================================================================

struct bot : public tinyfsm::MealyMachine<bot> {
  static std::shared_ptr<generic_camera> camera;
  static std::shared_ptr<robot_rpc_manager> rpc_manager;
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

inline void bot::react(const reset_event&) {
  LOG_WARNING(logger, "[{}::react] going to idle state after [reset_event]", to_string(get_state()));
  transit<idle_state>();
}

inline void bot::react(const terminate_event&) {
  LOG_WARNING(logger, "[{}::react] going to terminated state after [terminate_event]", to_string(get_state()));
  transit<terminated_state>();
}

inline void bot::react(const generic_event&) {
  // Default event handler
  LOG_WARNING(logger, "[{}::react] cannot handle event [generic_event]", to_string(get_state()));
}

inline void bot::react(const timeout_event&) {
  LOG_WARNING(logger, "[{}::react] cannot handle event [timeout_event]", to_string(get_state()));
}

inline void bot::react(const network_error_event&) {
  LOG_WARNING(logger, "[{}::react] cannot handle event [network_error_event]", to_string(get_state()));
}

inline void bot::react(const playback_error_event&) {
  LOG_WARNING(logger, "[{}::react] cannot handle event [playback_error_event]", to_string(get_state()));
}

inline void bot::react(const server_ready_event&) {
  LOG_WARNING(logger, "[{}::react] cannot handle event [server_ready_event]", to_string(get_state()));
}

inline void bot::react(const init_success_event&) {
  LOG_WARNING(logger, "[{}::react] cannot handle event [init_success_event]", to_string(get_state()));
}

inline void bot::react(const camera_error_event&) {
  LOG_WARNING(logger, "[{}::react] cannot handle event [camera_error_event]", to_string(get_state()));
}

inline void bot::react(const human_presence_event&) {
  LOG_WARNING(logger, "[{}::react] cannot handle event [human_presence_event]", to_string(get_state()));
}

inline void bot::react(const facial_recognition_response_event&) {
  LOG_WARNING(logger, "[{}::react] cannot handle event [facial_recognition_response_event]", to_string(get_state()));
}

inline void bot::react(const greeting_success_event&) {
  LOG_WARNING(logger, "[{}::react] cannot handle event [greeting_success_event]", to_string(get_state()));
}

inline void bot::react(const greeting_failure_event&) {
  LOG_WARNING(logger, "[{}::react] cannot handle event [greeting_failure_event]", to_string(get_state()));
}

inline void bot::react(const user_speech_detected_event&) {
  LOG_WARNING(logger, "[{}::react] cannot handle event [user_speech_detected_event]", to_string(get_state()));
}

inline void bot::react(const stream_speech_success_event&) {
  LOG_WARNING(logger, "[{}::react] cannot handle event [stream_speech_success_event]", to_string(get_state()));
}

inline void bot::react(const stream_speech_failure_event&) {
  LOG_WARNING(logger, "[{}::react] cannot handle event [stream_speech_failure_event]", to_string(get_state()));
}

inline void bot::react(const stream_response_success_event&) {
  LOG_WARNING(logger, "[{}::react] cannot handle event [stream_response_success_event]", to_string(get_state()));
}

inline void bot::react(const stream_response_failure_event&) {
  LOG_WARNING(logger, "[{}::react] cannot handle event [stream_response_failure_event]", to_string(get_state()));
}

inline void bot::entry() {}
inline void bot::exit() {}

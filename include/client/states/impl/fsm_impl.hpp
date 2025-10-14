#pragma once

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

#include "common/chat_utils.hpp"

//=============================================================================
// STATE MACHINE DECLARATIONS
//=============================================================================

// Forward declarations to avoid circular dependencies
class generic_camera;
class robot_rpc_manager;

struct bot : public tinyfsm::MealyMachine<bot> {
 public:
  static std::shared_ptr<generic_camera> camera;
  static std::shared_ptr<robot_rpc_manager> rpc_manager;

 protected:
  std::string current_sid;

 public:
  virtual void react(const reset_event&);
  virtual void react(const terminate_event&);
  virtual void react(const generic_event&);
  virtual void react(const timeout_event&);
  virtual void react(const network_error_event&);
  virtual void react(const playback_error_event&);
  virtual void react(const init_success_event&);
  virtual void react(const camera_error_event&);
  virtual void react(const human_presence_event&);
  virtual void react(const fr_success_event&);
  virtual void react(const greeting_success_event&);
  virtual void react(const vad_success_event&);
  virtual void react(const speech_success_event&);
  virtual void react(const response_success_event&);

  virtual void entry();
  virtual void exit();
  virtual client_state get_state() const = 0;
};

//=============================================================================
// STATE MACHINE DEFINITIONS
//=============================================================================

inline void bot::react(const reset_event&) {
  LOG_DEBUG(logger, "[{}::react] going to idle state after [reset_event]", to_string(get_state()));
  transit<idle_state>();
}

inline void bot::react(const terminate_event&) {
  LOG_DEBUG(logger, "[{}::react] going to terminated state after [terminate_event]", to_string(get_state()));
  transit<terminated_state>();
}

inline void bot::react(const generic_event&) {
  // Default event handler
  LOG_DEBUG(logger, "[{}::react] cannot handle event [generic_event]", to_string(get_state()));
}

inline void bot::react(const timeout_event&) {
  LOG_DEBUG(logger, "[{}::react] cannot handle event [timeout_event]", to_string(get_state()));
}

inline void bot::react(const network_error_event&) {
  LOG_DEBUG(logger, "[{}::react] cannot handle event [network_error_event]", to_string(get_state()));
}

inline void bot::react(const playback_error_event&) {
  LOG_DEBUG(logger, "[{}::react] cannot handle event [playback_error_event]", to_string(get_state()));
}

inline void bot::react(const init_success_event&) {
  LOG_DEBUG(logger, "[{}::react] cannot handle event [init_success_event]", to_string(get_state()));
}

inline void bot::react(const camera_error_event&) {
  LOG_DEBUG(logger, "[{}::react] cannot handle event [camera_error_event]", to_string(get_state()));
}

inline void bot::react(const human_presence_event&) {
  // LOG_DEBUG(logger, "[{}::react] cannot handle event [human_presence_event]", to_string(get_state()));
}

inline void bot::react(const fr_success_event&) {
  LOG_DEBUG(logger, "[{}::react] cannot handle event [fr_success_event]", to_string(get_state()));
}

inline void bot::react(const greeting_success_event&) {
  LOG_DEBUG(logger, "[{}::react] cannot handle event [greeting_success_event]", to_string(get_state()));
}

inline void bot::react(const vad_success_event&) {
  LOG_DEBUG(logger, "[{}::react] cannot handle event [vad_success_event]", to_string(get_state()));
}

inline void bot::react(const speech_success_event&) {
  LOG_DEBUG(logger, "[{}::react] cannot handle event [speech_success_event]", to_string(get_state()));
}

inline void bot::react(const response_success_event&) {
  LOG_DEBUG(logger, "[{}::react] cannot handle event [response_success_event]", to_string(get_state()));
}

inline void bot::entry() {}
inline void bot::exit() {}

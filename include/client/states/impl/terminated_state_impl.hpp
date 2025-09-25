#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct terminated_state final : robot {
  auto entry() -> void override { LOG_INFO(logger, "[{}::entry] Entering terminated state", to_string(get_state())); }
  auto get_state() const -> client_state override { return client_state::TERMINATED; }
};
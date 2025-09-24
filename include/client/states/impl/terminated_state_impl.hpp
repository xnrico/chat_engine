#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct terminated_state final : robot {
  auto get_state() const -> client_state override { return client_state::TERMINATED; }
  auto entry() -> void override { LOG_INFO(logger, "[terminated::entry] Entering terminated state"); }
};
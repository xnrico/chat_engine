#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct recognition_state final : bot {
  auto entry() -> void override {}

  auto react(const fr_success_event& e) -> void override { transit<greeting_state>(); }
  auto react(const timeout_event& e) -> void override { transit<fault_state>(); }
  auto react(const network_error_event& e) -> void override { transit<fault_state>(); }
  auto get_state() const -> client_state override { return client_state::RECOGNITION; }
};
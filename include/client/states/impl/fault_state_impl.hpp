#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct fault_state final : robot {
  auto entry() -> void override {
    LOG_WARNING(logger, "[fault::entry] Entering fault state, performing cleanup and logging");
    // Perform cleanup and logging here
    // After handling the fault, automatically transition back to idle state
    transit<idle_state>([]() -> void { LOG_WARNING(logger, "[fault::entry] Recovering to idle state"); });
  }

  auto get_state() const -> client_state override { return client_state::FAULT; }
};

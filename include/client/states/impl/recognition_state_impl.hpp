#pragma once

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <client/states/client_states.hpp>
#include <tinyfsm/tinyfsm.hpp>

//=============================================================================
// STATE DEFINITIONS
//=============================================================================

struct recognition_state final : bot {
  auto entry() -> void override {
    LOG_DEBUG(logger, "[{}::entry] Entering recognition state", to_string(get_state()));
    // Smart pointer with custom deleter to ensure session is cancelled on deletion
    auto uid = generate_id();
    bot::rpc_manager->sessions_map_.emplace(
        uid, std::shared_ptr<remote_session>{
                 // First argument: remote_session object created on heap
                 new remote_session{.session_id = uid,
                                    .request_time = std::chrono::steady_clock::now(),
                                    .target = remote_target::FR,
                                    .future = std::async(std::launch::async,
                                                         [uid]() {
                                                           bot::rpc_manager->init_fr_request(
                                                               uid, 5UL, []() { bot::dispatch(fr_success_event{}); },
                                                               []() { bot::dispatch(network_error_event{}); },
                                                               []() { bot::dispatch(timeout_event{}); });
                                                         })},
                 // Second argument: custom deleter lambda function
                 [this](remote_session* p) {
                   // Custom deleter to cancel the camera session when the shared_ptr
                   // goes out of scope
                   if (!p) return;
                   if (bot::rpc_manager) {
                     auto request = fr::fr_request_message{};
                     request.set_session_id(p->session_id);
                     bot::rpc_manager->cancel_fr_request(p->session_id);
                   }

                   delete p;
                 }});  // Custom deleter
  }

  auto react(const fr_success_event& e) -> void override { transit<greeting_state>(); }
  auto react(const timeout_event& e) -> void override { transit<fault_state>(); }
  auto react(const network_error_event& e) -> void override { transit<fault_state>(); }
  auto get_state() const -> client_state override { return client_state::RECOGNITION; }
};
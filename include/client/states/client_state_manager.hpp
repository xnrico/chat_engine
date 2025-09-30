#pragma once

#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/sinks/ConsoleSink.h>

#include <memory>

#include "client_states.hpp"

// All calls to state transitions should be made through this manager
class client_state_manager {
 private:
  std::shared_ptr<generic_camera> camera;
  std::shared_ptr<robot_rpc_manager> rpc_manager;
  std::shared_ptr<quill::Logger> logger;

  client_state_manager()
      : camera{std::make_shared<laptop_camera>()},
        rpc_manager{std::make_shared<robot_rpc_manager>()},
        logger{quill::Frontend::create_or_get_logger(
            getenv("USER") ? getenv("USER") : "unknown_user",
            quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_csm"))} {
    // logger configuration
    quill::Backend::start();
    logger->set_log_level(quill::LogLevel::TraceL3);

    bot::camera = camera;
    bot::logger = logger;
    bot::rpc_manager = rpc_manager;
  }

  ~client_state_manager();

 public:
  static auto get_instance() -> client_state_manager& {
    static client_state_manager instance;
    return instance;
  }

  auto start() -> void { bot::start(); }
};

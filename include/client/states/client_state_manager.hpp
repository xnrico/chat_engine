#pragma once

#include <memory>

#include "client_states.hpp"
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/sinks/ConsoleSink.h>

// All calls to state transitions should be made through this manager
class client_state_manager {
 private:
  std::shared_ptr<generic_camera> camera;
  std::shared_ptr<rtc_client> client;
  std::shared_ptr<quill::Logger> logger;

  client_state_manager()
      : camera{std::make_shared<laptop_camera>()},
        client{std::make_shared<rtc_client>()},
        logger{quill::Frontend::create_or_get_logger(
            getenv("USER") ? getenv("USER") : "unknown_user",
            quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_csm"))} {
    // logger configuration
    quill::Backend::start();
    logger->set_log_level(quill::LogLevel::TraceL3);

    robot::camera = camera;
    robot::client = client;
    robot::logger = logger;
  }

  ~client_state_manager();

 public:
  static auto get_instance() -> client_state_manager& {
    static client_state_manager instance;
    return instance;
  }

  auto start() -> void { robot::start(); }
};

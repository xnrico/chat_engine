#pragma once

#include <chrono>
#include <future>
#include <string>

enum struct remote_target { ROBOT, SERVER, FR };

struct remote_session {
  using time_point = std::chrono::time_point<std::chrono::steady_clock>;

  std::string session_id;    // unique session identifier
  time_point request_time;   // start time
  remote_target target;      // target type
  std::future<void> future;  // to signal completion
};
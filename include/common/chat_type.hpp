#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <rtc/rtc.hpp>

struct rtc_session {
  std::string session_id;
  std::shared_ptr<rtc::PeerConnection> pc;
  std::atomic<bool> active;
};
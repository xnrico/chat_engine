#pragma once

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <rtc/rtc.hpp>
#include <stdexcept>
#include <thread>
#include <unordered_set>
#include <vector>

#include "common/chat_utils.hpp"
#include "common/sessions/base_session.hpp"
#include "grpc/robot.grpc.pb.h"
#include "grpc/server.grpc.pb.h"

// Receiving RTP packets from a peer connection and forwarding to Facial Recognition engine

using namespace std::chrono_literals;

class camera_receiver final : public base_session {
 private:
  std::shared_ptr<robot::robot_service::Stub> stub;
  rtc::Configuration config{};  // customize (STUN/TURN) as needed
  std::shared_ptr<rtc::PeerConnection> pc;
  std::shared_ptr<rtc::RtcpReceivingSession> rtcp_session;
  std::vector<std::shared_ptr<rtc::Track>> tracks;

  std::thread watchdog_thread;
  std::atomic<bool> watchdog_running;
  std::atomic<std::chrono::steady_clock::time_point> last_packet_time;

 public:
  camera_receiver() = delete;
  camera_receiver(const std::string& sid, std::shared_ptr<robot::robot_service::Stub> stub);
  ~camera_receiver() override;

  std::string create_receiver(const std::string& offer_sdp);
};
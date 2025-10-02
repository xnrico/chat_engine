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

// Socket wrapper class for forwarding RTP packets to a peer connection

using namespace std::chrono_literals;

class camera_streamer final : public base_session {
 private:
  struct uplink {
    std::string session_id;
    std::shared_ptr<rtc::Track> track;
    std::function<void(std::array<char, 2048>, size_t)> on_data;  // pass by value
    std::function<void()> on_start;
    std::function<void()> on_camera_error;
    std::function<void()> on_timeout;

    uplink() = default;
  };

  struct capture {
    std::vector<uplink> uplinks;
    std::thread capture_thread;

    std::condition_variable capture_cv;
    std::mutex capture_mtx;

    int socket;
    std::atomic<bool> is_running;
    std::atomic<bool> has_data;

    capture() : socket{-1}, is_running{false}, has_data{false} {}
  };

 private:
  constexpr static size_t BUFFER_SIZE = 212992;  // max UDP packet size for RTP over IPv4
  constexpr static size_t SSRC = 42;             // arbitrary SSRC for the video track
  constexpr static size_t PAYLOAD_TYPE = 96;     // must match the payload type of the external h264 RTP stream
  constexpr static size_t TIMEOUT = 3;           // timeout to stop waiting for uplink to open
  static std::unordered_map<int, capture> captures;

 private:
  std::shared_ptr<server::server_service::Stub> stub;
  rtc::Configuration config{};  // customize (STUN/TURN) as needed
  std::shared_ptr<rtc::PeerConnection> pc;

  int rtp_port;

  // Callbacks
  std::function<void()> on_start;
  std::function<void()> on_server_error;
  std::function<void()> on_end;
  std::function<void()> on_camera_error;
  std::function<void()> on_timeout;

  void dispatch_uplink(const uplink &link);
  void capture_work(int port);

 public:
  camera_streamer() = delete;

  camera_streamer(const std::string &sid, int rtp_port, std::shared_ptr<server::server_service::Stub> stub)
      : base_session{sid}, rtp_port{rtp_port}, stub{stub} {}

  ~camera_streamer() override {
    // Release resources if no more uplinks
    if (captures[rtp_port].uplinks.empty()) {
      captures[rtp_port].is_running.store(false);
      if (captures[rtp_port].capture_thread.joinable()) {
        captures[rtp_port].capture_thread.join();
      }

      close(captures[rtp_port].socket);
      captures.erase(rtp_port);
      LOG_DEBUG(logger, "Camera stream on port {} destroyed", rtp_port);
    }
  }

  void remove_stream() {
    // This only removes the uplink from the capture list and marks the session inactive
    // The real cleanup is done in the destructor, which is called when the session is removed from the session manager
    // Linear search but should be ok since few uplinks are expected
    captures[rtp_port].uplinks.erase(
        std::remove_if(captures[rtp_port].uplinks.begin(), captures[rtp_port].uplinks.end(),
                       [this](const uplink &u) { return u.session_id == session_id; }),
        captures[rtp_port].uplinks.end());

    session_active.store(false);
    LOG_DEBUG(logger, "Camera stream for session {} marked inactive", session_id);
  }

  void create_stream() {
    pc = std::make_shared<rtc::PeerConnection>(config);

    auto media = rtc::Description::Video("video", rtc::Description::Direction::SendOnly);
    media.addH264Codec(96);  // Must match the payload type of the external h264 RTP stream
    media.addSSRC(SSRC, "video-send");
    auto track = pc->addTrack(media);

    // Set up peer connection event handlers
    pc->onGatheringStateChange([this](rtc::PeerConnection::GatheringState state) {
      // Only proceed when ICE gathering is complete
      if (state == rtc::PeerConnection::GatheringState::Complete) {
        auto offer = pc->localDescription();

        if (!offer) {
          LOG_ERROR(logger, "No local description available at ICE gathering complete stage");
          on_server_error();
          return;  // Cannot proceed without SDP
        }

        // Send the offer to the signaling server and wait for the answer
        grpc::ClientContext context;  // Must be non-null
        auto request = std::make_shared<server::init_camera_offer>();
        auto response = std::make_shared<server::init_camera_answer>();

        request->set_session_id(session_id);
        request->set_sdp(std::string{offer.value()});

        auto status = stub->init_camera_stream(&context, *request, response.get());

        if (status.ok()) {
          try {
            pc->setRemoteDescription(response->sdp());
          } catch (const std::exception &e) {
            LOG_ERROR(logger, "Failed to set remote description: {}", e.what());
            on_server_error();
          }
        } else {
          LOG_ERROR(logger, "Error {}, {}", static_cast<int>(status.error_code()), status.error_message());
          on_server_error();
        }
      }
    });

    pc->onStateChange([this, track](rtc::PeerConnection::State state) -> void {
      if (state == rtc::PeerConnection::State::Connected) {
        // Start streaming
        auto link = uplink{.session_id = session_id,
                           .track = track,
                           .on_data =
                               [this, track](std::array<char, 2048> buffer, size_t len) {
                                 auto rtp = reinterpret_cast<rtc::RtpHeader *>(buffer.data());
                                 rtp->setSsrc(SSRC);
                                 track->send(reinterpret_cast<const std::byte *>(buffer.data()), len);
                               },
                           .on_start = on_start,
                           .on_camera_error = on_camera_error,
                           .on_timeout = on_timeout};
        dispatch_uplink(link);
      } else if (state == rtc::PeerConnection::State::Disconnected) {
        // Error
        on_server_error();
      } else if (state == rtc::PeerConnection::State::Closed) {
        // Terminated by server (finished)
      }
    });

    pc->setLocalDescription(rtc::Description::Type::Offer);
  }

  void set_on_start(std::function<void()> callback) { on_start = std::move(callback); }
  void set_on_server_error(std::function<void()> callback) { on_server_error = std::move(callback); }
  void set_on_end(std::function<void()> callback) { on_end = std::move(callback); }
  void set_on_camera_error(std::function<void()> callback) { on_camera_error = std::move(callback); }
  void set_on_timeout(std::function<void()> callback) { on_timeout = std::move(callback); }
};
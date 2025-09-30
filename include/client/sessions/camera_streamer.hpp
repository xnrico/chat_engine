#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/Logger.h>
#include <quill/sinks/ConsoleSink.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <memory>
#include <rtc/rtc.hpp>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <unordered_set>
#include <vector>

#include <grpcpp/grpcpp.h>
#include "grpc/robot.grpc.pb.h"
#include "grpc/server.grpc.pb.h"

#include "base_session.hpp"

// Socket wrapper class for forwarding RTP packets to a peer connection

class camera_streamer final : public base_session {
 private:
  constexpr static size_t BUFFER_SIZE = 212992;  // max UDP packet size for RTP over IPv4
  constexpr static size_t SSRC = 42;             // arbitrary SSRC for the video track
  constexpr static size_t PAYLOAD_TYPE = 96;     // must match the payload type of the external h264 RTP stream

  // Media description is shared by the class
  static std::unordered_map<int, std::tuple<std::shared_ptr<rtc::Description::Video>, int, int>>
      media_map;  // map media port to [description, socket, freq]

  // Media description for the video track
  std::shared_ptr<rtc::Description::Video> media;
  int port;
  int sock;

  std::shared_ptr<server::server_service::Stub> stub;
  rtc::Configuration config{};  // customize (STUN/TURN) as needed
  std::shared_ptr<rtc::PeerConnection> pc;
  std::shared_ptr<rtc::Track> uplink;

  std::shared_ptr<quill::Logger> logger;
  std::thread stream_thread;
  std::atomic<bool> is_running;

  void stream() {
    constexpr static size_t STREAM_BUF_SIZE = 2048;
    char buffer[STREAM_BUF_SIZE];
    int len{};

    while (is_running.load() && (len = recv(sock, buffer, STREAM_BUF_SIZE, 0)) >= 0) {
      // Receive from UDP

      if (len < sizeof(rtc::RtpHeader) || !uplink->isOpen()) continue;

      auto rtp = reinterpret_cast<rtc::RtpHeader *>(buffer);
      rtp->setSsrc(SSRC);

      uplink->send(reinterpret_cast<const std::byte *>(buffer), len);
    }

    is_running.store(false);
  }

 public:
  camera_streamer() = delete;

  camera_streamer(const std::string &sid, int rtp_port, std::shared_ptr<server::server_service::Stub> stub)
      : base_session{sid},
        port{rtp_port},
        sock{-1},
        media{nullptr},
        pc{nullptr},
        uplink{nullptr},
        is_running{false},
        stub{stub},
        logger{quill::Frontend::create_or_get_logger(
            getenv("USER") ? getenv("USER") : "unknown_user",
            quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_camera_streamer_" + session_id))} {
    // Start logger
    quill::Backend::start();

    // create media description
    auto it = media_map.find(rtp_port);
    if (it != media_map.end()) {
      media = std::get<0>(it->second);
      sock = std::get<1>(it->second);
      ++std::get<2>(it->second);  // reference counter
    } else {
      media = std::make_shared<rtc::Description::Video>("video", rtc::Description::Direction::SendOnly);
      sock = socket(AF_INET, SOCK_DGRAM, 0);

      struct sockaddr_in addr = {};
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = inet_addr("127.0.0.1");
      addr.sin_port = htons(rtp_port);

      if (bind(sock, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) < 0) {
        ::close(sock);
        throw std::runtime_error("Failed to bind UDP socket on 127.0.0.1:" + std::to_string(rtp_port));
      }

      setsockopt(sock, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char *>(&BUFFER_SIZE), sizeof(BUFFER_SIZE));

      media->addH264Codec(PAYLOAD_TYPE);  // Must match the payload type of the external h264 RTP stream
      media->addSSRC(SSRC, "video-send");

      media_map.try_emplace(rtp_port, std::make_tuple(media, sock, 1));
    }
  }

  ~camera_streamer() override {
    is_running.store(false);
    if (stream_thread.joinable()) stream_thread.join();

    auto it = media_map.find(port);

    if (--std::get<2>(it->second) == 0) {
      media_map.erase(port);
    }

    close(sock);
  }

  void create_stream() {
    pc = std::make_shared<rtc::PeerConnection>(config);

    // Set up peer connection event handlers
    pc->onGatheringStateChange([this](rtc::PeerConnection::GatheringState state) {
      // Only proceed when ICE gathering is complete
      if (state == rtc::PeerConnection::GatheringState::Complete) {
        auto offer = pc->localDescription();

        if (!offer) {
          LOG_ERROR(logger, "No local description available at ICE gathering complete stage");
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
          }
        } else {
          LOG_ERROR(logger, "Error {}, {}", static_cast<int>(status.error_code()), status.error_message());
        }
      }
    });

    pc->onStateChange([this](rtc::PeerConnection::State state) -> void {
      if (state == rtc::PeerConnection::State::Connected) {
        // Start streaming
        is_running.store(true);
        stream_thread = std::thread([this]() -> void { stream(); });
      } else if (state == rtc::PeerConnection::State::Disconnected) {
        // Error
        is_running.store(false);
      } else if (state == rtc::PeerConnection::State::Closed) {
        // Terminated by server (finished)
        is_running.store(false);
      }
    });

    uplink = pc->addTrack(*media);
    // pc->createOffer();
    pc->setLocalDescription(rtc::Description::Type::Offer);
  }
};
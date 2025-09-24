#include "client/rpc/rtc_client.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/Logger.h>
#include <quill/sinks/ConsoleSink.h>
#include <sys/socket.h>

#include <nlohmann/json.hpp>
#include <random>

#include "grpc/chat.grpc.pb.h"
#include "grpc/signaling.grpc.pb.h"

rtc_client::rtc_client()
    : logger{quill::Frontend::create_or_get_logger(
          getenv("USER") ? getenv("USER") : "unknown_user",
          quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_rtc"))},
      channel{grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials())},
      signaling_stub{signaling::signaling_service::NewStub(channel)},
      chat_stub{chat::chat_service::NewStub(channel)} {}

rtc_client::~rtc_client() {
  // Clean up resources
}

auto generate_id(size_t n = 8) -> std::string {  // generate random id of length n
  static thread_local std::mt19937 rng(
      static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
  static const std::string dictionary("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

  auto id = std::string(n, '0');
  auto uniform = std::uniform_int_distribution<int>(0, int(dictionary.size() - 1));
  std::generate(id.begin(), id.end(), [&]() { return dictionary.at(uniform(rng)); });
  return id;
}

auto rtc_client::create_pc(const std::string &remote_id) -> std::shared_ptr<rtc::PeerConnection> {
  auto pc = std::make_shared<rtc::PeerConnection>(config);

  pc->onStateChange([this](rtc::PeerConnection::State state) {
    LOG_DEBUG(logger, "PeerConnection state changed: {}", static_cast<int>(state));
  });

  pc->onGatheringStateChange([this, pc](rtc::PeerConnection::GatheringState state) {
    LOG_DEBUG(logger, "Gathering state changed: {}", static_cast<int>(state));
    if (state == rtc::PeerConnection::GatheringState::Complete) {
      LOG_DEBUG(logger, "ICE Gathering complete");
      auto description = pc->localDescription();
      LOG_DEBUG(logger, "Local description: {}", std::string{description.value()});

      auto request = signaling::offer_request{};
      auto response = std::make_shared<signaling::offer_response>();

      request.set_sdp(std::string{description.value()});

      LOG_DEBUG(logger, "Sent offer to signaling server");
      auto ret = signaling_stub->offer(nullptr, std::move(request), response.get());

      if (ret.ok()) {
        LOG_DEBUG(logger, "Received response from signaling server: {}", response->sdp());
        pc->setRemoteDescription(response->sdp());
      } else {
        LOG_ERROR(logger, "Failed to receive response from signaling server");
      }
    }
  });

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(6000);

  if (bind(sock, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) < 0)
    throw std::runtime_error("Failed to bind UDP socket on 127.0.0.1:6000");

  int rcvBufSize = 212992;
  setsockopt(sock, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char *>(&rcvBufSize), sizeof(rcvBufSize));

  const rtc::SSRC ssrc = 42;
  rtc::Description::Video media("video", rtc::Description::Direction::SendOnly);
  media.addH264Codec(96);  // Must match the payload type of the external h264 RTP stream
  media.addSSRC(ssrc, "video-send");
  auto track = pc->addTrack(media);

  pc->setLocalDescription();

  // std::atomic_bool running{true};

  peer_connections.emplace(remote_id, pc);
  return pc;
}

auto rtc_client::stream_media(int sock) -> void {
  // std::array<uint8_t, 2048> buf;
  // while (running) {
  //   ssize_t n = recv(sock, buf.data(), buf.size(), 0);
  //   if (n <= 0) continue;
  //   try {
  //     auto pkt = rtc::RtpPacket::parse(buf.data(), size_t(n));
  //     track->send(std::move(pkt));
  //   } catch (const std::exception &e) {
  //     LOG_ERROR(logger, "RTP parse/send failed: {}", e.what());
  //   }
  // }
  // close(sock);
}

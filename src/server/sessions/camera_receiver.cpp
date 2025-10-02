#include "server/sessions/camera_receiver.hpp"

#include <chrono>

#include "common/chat_utils.hpp"

using namespace std::chrono_literals;

camera_receiver::camera_receiver(const std::string& sid, std::shared_ptr<robot::robot_service::Stub> stub)
    : base_session{sid}, stub{stub}, pc{nullptr}, watchdog_running{false} {
  watchdog_running.store(true);
//   watchdog_thread = std::thread([this]() {
//     while (watchdog_running.load()) {
//       if (!tracks.empty() && std::chrono::steady_clock::now() - last_packet_time.load() > 1s) {
//         LOG_WARNING(logger, "No RTP packets received for 1 second, marking session inactive");
//         session_active.store(false);
//       }
//     }
//   });
// }

camera_receiver::~camera_receiver() {
  watchdog_running.store(false);
  if (watchdog_thread.joinable()) {
    watchdog_thread.join();
  }
}

std::string camera_receiver::create_receiver(const std::string& offer_sdp) {
  // Synchronization primitives
  auto answer_sdp = std::string{};
  auto cv = std::condition_variable{};
  auto cv_mtx = std::mutex{};
  pc = std::make_shared<rtc::PeerConnection>(config);  // Create a new PeerConnection

  // set up callbacks
  pc->onGatheringStateChange(
      [this](rtc::PeerConnection::GatheringState s) { LOG_DEBUG(logger, "Gathering state: {}", static_cast<int>(s)); });
  pc->onLocalDescription([this](rtc::Description desc) {
    LOG_DEBUG(logger, "Local description set: type={} size={} bytes", desc.typeString(), desc.generateSdp().size());
  });
  pc->onGatheringStateChange([&](rtc::PeerConnection::GatheringState state) {
    if (state == rtc::PeerConnection::GatheringState::Complete) {
      auto answer = pc->localDescription();
      if (answer.has_value()) {
        answer_sdp = answer->generateSdp();
        LOG_DEBUG(logger, "ICE gathering complete, answer SDP size: {}", answer_sdp.size());
        cv.notify_all();
      } else {
        LOG_ERROR(logger, "Local description not set at gathering complete");
      }
    }
  });

  rtc::Description::Video media("video", rtc::Description::Direction::RecvOnly);
  media.addH264Codec(96);
  media.setBitrate(3000);  // Request 3Mbps (Browsers do not encode more than 2.5MBps from a webcam)

  auto track = pc->addTrack(media);

  rtcp_session = std::make_shared<rtc::RtcpReceivingSession>();
  track->setMediaHandler(rtcp_session);
  track->onMessage(
      [this](rtc::binary message) {
        // This is an RTP packet
        LOG_DEBUG(logger, "Received RTP packet of size {} on session", message.size());
        last_packet_time.store(std::chrono::steady_clock::now());
      },
      nullptr);
  track->onOpen([this] { LOG_DEBUG(logger, "track opened"); });
  track->onClosed([this] {
    session_active.store(false);
    LOG_DEBUG(logger, "track closed");
  });

  // Make tracks persistent to avoid being GC'd
  tracks.emplace_back(std::move(track));

  // Execution steps:
  // 1. Set remote description (offer from client)
  try {
    pc->setRemoteDescription(offer_sdp);
  } catch (const std::exception& e) {
    LOG_ERROR(logger, "Failed to set remote description: {}", e.what());
    return std::string{};
  }

  // 2. Create and set local description (answer)
  try {
    auto answer = pc->createAnswer();  // calls pc->setLocalDescription() internally
  } catch (const std::exception& e) {
    return std::string{};
  }

  // 3. Wait for ICE gathering to complete (with timeout)
  {
    auto lock = std::unique_lock<std::mutex>(cv_mtx);
    if (!cv.wait_for(lock, 3s, [&]() { return !answer_sdp.empty(); })) {
      LOG_ERROR(logger, "Timeout waiting for ICE gathering to complete");
      return std::string{};
    }
  }

  return answer_sdp;
}
#include "server/sessions/camera_receiver.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>

#include <chrono>
#include <vector>

#include "common/chat_utils.hpp"

using namespace std::chrono_literals;

static std::mutex cin_mtx;  // to synchronize console input (testing)

camera_receiver::camera_receiver(const std::string& sid, std::shared_ptr<robot::robot_service::Stub> stub)
    : base_session{sid}, stub{stub}, pc{nullptr}, watchdog_running{false} {}

camera_receiver::~camera_receiver() {
  watchdog_running.store(false);
  if (watchdog_thread.joinable()) {
    watchdog_thread.join();
  }

  ::close(sock);
}

std::string camera_receiver::create_receiver(const std::string& offer_sdp) {
  // Synchronization primitives

  pc = std::make_shared<rtc::PeerConnection>(config);  // Create a new PeerConnection

  // set up callbacks
  pc->onGatheringStateChange([this](rtc::PeerConnection::GatheringState state) {
    if (state == rtc::PeerConnection::GatheringState::Complete) {
      auto answer = pc->localDescription();
      if (answer.has_value()) {
        answer_sdp = answer->generateSdp();
        cv.notify_all();
      }
    }
  });

  auto media = rtc::Description::Video("video", rtc::Description::Direction::RecvOnly);
  media.addH264Codec(96);
  media.setBitrate(5000);  // Request 5Mbps (Browsers do not encode more than 2.5MBps from a webcam)

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  sockaddr_in rtp_addr{};
  rtp_addr.sin_family = AF_INET;
  rtp_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  rtp_addr.sin_port = htons(7000);  // Port where the Facial Recognition engine listens for RTP

  track = pc->addTrack(media);

  rtcp_session = std::make_shared<rtc::RtcpReceivingSession>();
  track->setMediaHandler(rtcp_session);
  track->onMessage(
      [this, rtp_addr](rtc::binary message) {
        // This is an RTP packet
        last_packet_time.store(std::chrono::steady_clock::now());
        sendto(sock, reinterpret_cast<const char*>(message.data()), int(message.size()), 0,
               reinterpret_cast<const struct sockaddr*>(&rtp_addr), sizeof(rtp_addr));
      },
      [this](const std::string& error) {
        // This is an RTP error packet
        LOG_DEBUG(logger, "Received RTP packet error: {}", error);
      });
  track->onOpen([this] {
    watchdog_running.store(true);

    // std::thread([this]() -> void {
    //   auto fds = std::vector<pollfd>{{STDIN_FILENO, POLLIN, 0}};
    //   LOG_DEBUG(logger, "Track opened on session {}, started receiving", session_id);
    //   LOG_DEBUG(logger, "Please enter greeting response within 5 seconds... Greeted? (y/n): ");

    //   std::unique_lock<std::mutex> lock(cin_mtx);
    //   int ret = poll(fds.data(), fds.size(), 5000);  // 5 seconds timeout (basically cin with timeout)
    //   char response{};
    //   if (ret > 0) {
    //     std::cin >> response;
    //     lock.unlock();

    //     if (response == 'y' || response == 'Y') {
    //       LOG_DEBUG(logger, "Greeted...");
    //       greeting_promise.set_value(true);
    //     } else {
    //       LOG_DEBUG(logger, "Not greeted...");
    //       session_active.store(false);
    //       greeting_promise.set_value(false);
    //     }
    //   }
    // }).detach();

    watchdog_thread = std::thread([this]() {
      while (session_active.load()) {
        std::this_thread::sleep_for(500ms);

        auto now = std::chrono::steady_clock::now();
        auto last_packet = last_packet_time.load();

        // Check if we haven't received any packets for 500ms
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_packet).count() > 500) {
          // Close the track to trigger cleanup
          if (track) {
            track->close();
          }
          session_active.store(false);
          break;
        }
      }
    });
  });
  track->onClosed([this] {
    session_active.store(false);
  });

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
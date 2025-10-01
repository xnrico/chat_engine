#include "server/server_rpc_manager.hpp"

// logging includes
#include <grpcpp/grpcpp.h>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/Logger.h>
#include <quill/sinks/ConsoleSink.h>

#include <chrono>
#include <condition_variable>
#include <mutex>

#include "common/chat_utils.hpp"

using namespace std::chrono_literals;

server_rpc_manager::server_rpc_manager()
    : logger{quill::Frontend::create_or_get_logger(
          getenv("USER") ? getenv("USER") : "unknown_user",
          quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_server_rpc"))},
      channel{grpc::CreateChannel("localhost:6002", grpc::InsecureChannelCredentials())},
      stub{robot::robot_service::NewStub(channel)} {
  // Constructor body (if needed)
  quill::Backend::start();
}

grpc::Status server_rpc_manager::init_camera_stream(grpc::ServerContext* context,
                                                    const server::init_camera_offer* request,
                                                    server::init_camera_answer* response) {
  // Implementation of the method
  if (!request || request->sdp().empty()) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty offer SDP");
  }

  if (sessions.size() >= MAX_SESSIONS) {
    LOG_WARNING(logger, "Max sessions reached, cannot create new session");
    return grpc::Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "Max sessions reached");
  }

  // Synchronization primitives
  auto answer_sdp = std::string{};
  auto cv = std::condition_variable{};
  auto cv_mtx = std::mutex{};
  auto pc = std::make_shared<rtc::PeerConnection>(config);  // Create a new PeerConnection

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
        cv.notify_all();
      } else {
        LOG_ERROR(logger, "Local description not set at gathering complete");
      }
    }
  });

  rtc::Description::Video media("video", rtc::Description::Direction::RecvOnly);
  media.addH264Codec(96);
  // media.setBitrate(3000);  // Request 3Mbps (Browsers do not encode more than 2.5MBps from a webcam)

  auto track = pc->addTrack(media);

  auto session = std::make_shared<rtc::RtcpReceivingSession>();
  track->setMediaHandler(session);
  track->onMessage(
      [this, session](rtc::binary message) {
        // This is an RTP packet
        LOG_DEBUG(logger, "Received RTP packet of size {} on session", message.size());
      },
      nullptr);
  track->onOpen([this] { LOG_DEBUG(logger, "track opened"); });

  // Execution steps:
  // 1. Set remote description (offer from client)
  try {
    pc->setRemoteDescription(request->sdp());
  } catch (const std::exception& e) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, std::string("Failed to set remote offer: ") + e.what());
  }

  // 2. Create and set local description (answer)
  try {
    auto answer = pc->createAnswer();  // calls pc->setLocalDescription() internally
  } catch (const std::exception& e) {
    return grpc::Status(grpc::StatusCode::INTERNAL, std::string("Failed to create local description: ") + e.what());
  }

  // 3. Wait for ICE gathering to complete (with timeout)
  {
    auto lock = std::unique_lock<std::mutex>(cv_mtx);
    if (!cv.wait_for(lock, 3s, [&]() { return !answer_sdp.empty(); })) {
      LOG_ERROR(logger, "Timeout waiting for ICE gathering to complete");
      return grpc::Status(grpc::StatusCode::DEADLINE_EXCEEDED, "Timeout waiting for ICE gathering to complete");
    }
  }

  // 4. Persist session (could be used later for trickle, stats, teardown)
  {
    auto lock = std::lock_guard<std::mutex>(mtx);
    sessions.emplace(request->session_id(), std::move(pc));
  }

  response->set_session_id(request->session_id());
  response->set_sdp(answer_sdp);
  return grpc::Status::OK;
}
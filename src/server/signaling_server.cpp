#include "server/signaling_server.hpp"

#include <grpcpp/grpcpp.h>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/Logger.h>
#include <quill/sinks/ConsoleSink.h>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <random>
#include <thread>

#include "grpc/signaling.grpc.pb.h"

using namespace std::chrono_literals;

signaling_server::signaling_server()
    : logger{
          quill::Frontend::create_or_get_logger(getenv("USER") ? getenv("USER") : "unknown_user",
                                                quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_signaling"))} {}
signaling_server::~signaling_server() {}

grpc::Status signaling_server::offer(grpc::ServerContext* /*context*/, const signaling::offer_request* request,
                                     signaling::offer_response* response) {
  if (!request || request->sdp().empty()) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty offer SDP");
  }
  // Create a PeerConnection, set remote description (the client's offer) and generate an answer.
  // Because this is a unary RPC we block until ICE gathering completes (or timeout) so that
  // the returned SDP contains all candidates (no trickle ICE yet).

  auto generate_id = []() -> std::string {
    static thread_local std::mt19937 rng(
        static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    static const std::string dict = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::uniform_int_distribution<int> dist(0, static_cast<int>(dict.size() - 1));
    std::string out(8, '0');
    for (auto& c : out) c = dict[dist(rng)];
    return out;
  };

  auto pc = std::make_shared<rtc::PeerConnection>(config);

  // Logging helpers
  pc->onStateChange([this](rtc::PeerConnection::State s) { LOG_INFO(logger, "PC state: {}", static_cast<int>(s)); });
  pc->onGatheringStateChange([this](rtc::PeerConnection::GatheringState s) {
    LOG_DEBUG(logger, "Gathering state: {}", static_cast<int>(s));
  });
  pc->onLocalDescription([this](rtc::Description desc) {
    LOG_DEBUG(logger, "Local description set: type={} size={} bytes", desc.typeString(), desc.generateSdp().size());
  });
  pc->onDataChannel([this](std::shared_ptr<rtc::DataChannel> dc) {
    LOG_INFO(logger, "Remote DataChannel label={}", dc->label());
    dc->onMessage([this, weak = std::weak_ptr<rtc::DataChannel>(dc)](rtc::message_variant msg) {
      if (auto locked = weak.lock()) {
        if (std::holds_alternative<std::string>(msg)) {
          LOG_DEBUG(logger, "Incoming message: {}", std::get<std::string>(msg));
        }
      }
    });
  });

  // Synchronization primitives to wait for ICE gathering completion.
  std::mutex cv_m;
  std::condition_variable cv;
  bool done = false;
  std::string answer_sdp;
  bool failed = false;
  std::string failure_reason;

  pc->onGatheringStateChange([&](rtc::PeerConnection::GatheringState state) {
    if (state == rtc::PeerConnection::GatheringState::Complete) {
      auto ld = pc->localDescription();
      if (ld.has_value()) {
        answer_sdp = ld->generateSdp();
      } else {
        failed = true;
        failure_reason = "Local description not set at gathering complete";
      }
      {
        std::lock_guard<std::mutex> lk(cv_m);
        done = true;
      }
      cv.notify_all();
    }
  });

  // 1. Set remote description (offer from client)
  try {
    pc->setRemoteDescription(request->sdp());
  } catch (const std::exception& e) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, std::string("Failed to set remote offer: ") + e.what());
  }

  // 2. Create and set local description (answer)
  try {
    pc->setLocalDescription();
  } catch (const std::exception& e) {
    return grpc::Status(grpc::StatusCode::INTERNAL, std::string("Failed to create local description: ") + e.what());
  }

  // 3. Wait for ICE gathering completion or timeout (e.g., 3 seconds)
  {
    std::unique_lock<std::mutex> lk(cv_m);
    cv.wait_for(lk, std::chrono::seconds(3), [&] { return done; });
  }

  if (!done) {
    // Timed out: attempt to return whatever SDP we currently have (end-of-gathering not reached)
    auto ld = pc->localDescription();
    if (ld.has_value()) {
      answer_sdp = ld->generateSdp();
      LOG_WARNING(logger, "ICE gathering timeout – returning partial SDP ({} bytes)", answer_sdp.size());
    } else {
      return grpc::Status(grpc::StatusCode::DEADLINE_EXCEEDED, "Timed out gathering ICE candidates and no SDP");
    }
  } else if (failed) {
    return grpc::Status(grpc::StatusCode::INTERNAL, failure_reason);
  }

  // 4. Persist session (could be used later for trickle, stats, teardown)
  const auto sid = generate_id();
  {
    std::lock_guard<std::mutex> lock(mtx);
    sessions.emplace(sid, session{pc, std::chrono::steady_clock::now()});
  }

  response->set_sdp(answer_sdp);
  return grpc::Status::OK;
}
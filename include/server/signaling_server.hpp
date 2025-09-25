#pragma once

#include <grpc/signaling.grpc.pb.h>
#include <grpcpp/grpcpp.h>
#include <quill/Logger.h>

#include <memory>
#include <mutex>
#include <optional>
#include <rtc/rtc.hpp>
#include <unordered_map>

// Basic unary gRPC signaling: client sends Offer SDP, server creates PeerConnection,
// sets remote description, generates Answer SDP and returns it directly.
// (ICE trickle can be added later via a streaming RPC if needed.)

class signaling_server final : public signaling::signaling_service::Service {
 private:
  struct session {
    std::shared_ptr<rtc::PeerConnection> pc;
    std::chrono::steady_clock::time_point created_at;
  };

  std::shared_ptr<quill::Logger> logger;
  mutable std::mutex mtx;
  std::unordered_map<std::string, session> sessions;  // key could be future client id
  rtc::Configuration config{};                        // customize (STUN/TURN) as needed

 public:
  signaling_server();
  ~signaling_server();

  grpc::Status offer(grpc::ServerContext* context, const signaling::offer_request* request,
                     signaling::offer_response* response) override;
};
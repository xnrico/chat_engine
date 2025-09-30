#pragma once
#include <grpcpp/grpcpp.h>
#include <quill/Logger.h>

#include <memory>
#include <mutex>
#include <rtc/rtc.hpp>

#include "grpc/robot.grpc.pb.h"
#include "grpc/server.grpc.pb.h"

#include "common/chat_type.hpp"

class server_rpc_manager final : public server::server_service::Service {
 private:
  constexpr static size_t MAX_SESSIONS = 10;

  std::shared_ptr<quill::Logger> logger;
  std::shared_ptr<rtc::PeerConnection> pc;
  rtc::Configuration config{};  // customize (STUN/TURN) as needed

  std::shared_ptr<grpc::Channel> channel;
  std::shared_ptr<robot::robot_service::Stub> stub;

  std::unordered_map<std::string, std::shared_ptr<rtc::PeerConnection>> sessions;
  mutable std::mutex mtx;  // mutex for the sessions map

 public:
  server_rpc_manager();
  ~server_rpc_manager() override = default;

  grpc::Status init_camera_stream(grpc::ServerContext* context, const server::init_camera_offer* request,
                                  server::init_camera_answer* response) override;
};
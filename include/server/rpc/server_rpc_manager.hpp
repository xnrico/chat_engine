#pragma once
#include <grpcpp/grpcpp.h>
#include <quill/Logger.h>

#include <functional>
#include <memory>
#include <mutex>
#include <rtc/rtc.hpp>

#include "common/chat_type.hpp"
#include "grpc/robot.grpc.pb.h"
#include "grpc/server.grpc.pb.h"
#include "common/sessions/base_session.hpp"

class server_rpc_manager final : public server::server_service::Service {
 private:
  constexpr static size_t MAX_SESSIONS = 10;
  std::shared_ptr<grpc::Channel> channel;
  std::shared_ptr<robot::robot_service::Stub> stub;

  std::unordered_map<std::string, std::shared_ptr<base_session>> sessions;
  mutable std::mutex mtx;  // mutex for the sessions map

  std::thread periodic_thread;
  std::atomic<bool> is_running;

  void cleanup_sessions();

 public:
  server_rpc_manager();
  ~server_rpc_manager() override;

  grpc::Status init_camera_stream(grpc::ServerContext* context, const server::init_camera_offer* request,
                                  server::init_camera_answer* response) override;
};
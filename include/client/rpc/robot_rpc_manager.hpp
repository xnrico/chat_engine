#pragma once
#include <grpcpp/grpcpp.h>
#include <quill/Logger.h>

#include <functional>
#include <memory>
#include <mutex>

#include "grpc/fr.grpc.pb.h"
#include "grpc/robot.grpc.pb.h"
#include "grpc/server.grpc.pb.h"

class robot_rpc_manager final : public robot::robot_service::Service {
 private:
  constexpr static size_t MAX_SESSIONS = 10;

 private:
  std::shared_ptr<grpc::Channel> channel;
  std::shared_ptr<server::server_service::Stub> server_stub;
  std::shared_ptr<fr::fr_service::Stub> fr_stub;

  mutable std::mutex mtx;  // to protect sessions map

  std::thread periodic_thread;
  std::atomic<bool> is_running;

 public:
  robot_rpc_manager();
  ~robot_rpc_manager() override;

  // Calls to the server
  std::string init_camera_stream();
  void stop_camera_session(const std::string& session_id);
};
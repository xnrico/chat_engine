#pragma once

#include <grpcpp/grpcpp.h>
#include <quill/Logger.h>

#include <functional>
#include <memory>
#include <mutex>

#include "grpc/fr.grpc.pb.h"
#include "grpc/robot.grpc.pb.h"
#include "grpc/server.grpc.pb.h"

class server_rpc_manager final : public server::server_service::Service {
 private:
  constexpr static size_t MAX_SESSIONS = 10;

 private:
  std::shared_ptr<grpc::Channel> channel;
  std::shared_ptr<robot::robot_service::Stub> robot_stub;
  std::shared_ptr<fr::fr_service::Stub> fr_stub;

  mutable std::mutex mtx;  // mutex for the sessions map

  std::thread periodic_thread;
  std::atomic<bool> is_running;


 public:
  server_rpc_manager();
  ~server_rpc_manager() override;
};
#pragma once
#include <grpcpp/grpcpp.h>
#include <quill/Logger.h>

#include <functional>
#include <memory>
#include <mutex>

#include "common/chat_types.hpp"
#include "common/fifo_map.hpp"
#include "grpc/fr.grpc.pb.h"
#include "grpc/robot.grpc.pb.h"
#include "grpc/server.grpc.pb.h"

class robot_rpc_manager final : public robot::robot_service::Service {
 private:
  constexpr static size_t MAX_SESSIONS = 8UL;  // must be power of 2

 private:
  std::shared_ptr<grpc::Channel> server_channel;
  std::shared_ptr<server::server_service::Stub> server_stub;
  std::shared_ptr<grpc::Channel> fr_channel;
  std::shared_ptr<fr::fr_service::Stub> fr_stub;

  std::thread periodic_thread;
  std::atomic<bool> is_running;

 public:
  robot_rpc_manager();
  ~robot_rpc_manager() override;

  fifo_map<std::string, std::shared_ptr<remote_session>> sessions_map_;  // thread safe

  // Calls to the server
  void init_fr_request(
      const std::string& session_id, size_t timeout = 5U, std::function<void()> on_success = []() {},
      std::function<void()> on_failure = []() {}, std::function<void()> on_timeout = []() {});  // 5s timeout
  void cancel_fr_request(
      const std::string& session_id, size_t timeout = 5U, std::function<void()> on_success = []() {},
      std::function<void()> on_failure = []() {}, std::function<void()> on_timeout = []() {});  // 5s timeout
};
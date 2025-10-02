#pragma once
#include <grpcpp/grpcpp.h>
#include <quill/Logger.h>

#include <functional>
#include <memory>
#include <mutex>
#include <rtc/rtc.hpp>

#include "client/sessions/base_session.hpp"
#include "common/chat_type.hpp"
#include "grpc/robot.grpc.pb.h"
#include "grpc/server.grpc.pb.h"

class robot_rpc_manager final : public robot::robot_service::Service {
 private:
  constexpr static size_t MAX_SESSIONS = 10;
  std::shared_ptr<grpc::Channel> channel;
  std::shared_ptr<server::server_service::Stub> stub;

  std::unordered_map<std::string, std::shared_ptr<base_session>> sessions;
  mutable std::mutex mtx;  // to protect sessions map

  std::thread periodic_thread;
  std::atomic<bool> is_running;

  void cleanup_sessions();

 public:
  robot_rpc_manager();
  ~robot_rpc_manager() override;

  grpc::Status stop_camera_stream(grpc::ServerContext* context, const robot::generic_message* request,
                                  robot::response_message* response) override;

  grpc::Status offer(grpc::ServerContext* context, const robot::offer_request* request,
                     robot::offer_response* response) override;

  std::string init_camera_stream(std::function<void()> on_start, std::function<void()> on_server_error,
                                 std::function<void()> on_camera_error, std::function<void()> on_timeout,
                                 std::function<void()> on_end);

  void stop_camera_stream(const std::string& session_id);
};
#include "client/rpc/robot_rpc_manager.hpp"

#include <grpcpp/grpcpp.h>

#include "common/chat_utils.hpp"

robot_rpc_manager::robot_rpc_manager()
    : channel{grpc::CreateChannel("localhost:6001", grpc::InsecureChannelCredentials())},
      server_stub{server::server_service::NewStub(channel)},
      fr_stub{fr::fr_service::NewStub(channel)} {
  // Constructor body (if needed)
}

robot_rpc_manager::~robot_rpc_manager() {
  is_running.store(false);
  if (periodic_thread.joinable()) {
    periodic_thread.join();
  }
}

std::string robot_rpc_manager::init_camera_stream() {
  // Implementation of the method
  auto sid = generate_id();
  return sid;
}

void robot_rpc_manager::stop_camera_session(const std::string& session_id) { static_assert(true); }
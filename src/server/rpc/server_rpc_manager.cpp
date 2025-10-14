#include "server/rpc/server_rpc_manager.hpp"

// logging includes
#include <grpcpp/grpcpp.h>

#include <chrono>
#include <condition_variable>
#include <mutex>

#include "common/chat_utils.hpp"

using namespace std::chrono_literals;

server_rpc_manager::server_rpc_manager()
    : channel{grpc::CreateChannel("localhost:6002", grpc::InsecureChannelCredentials())},
      robot_stub{robot::robot_service::NewStub(channel)},
      fr_stub{fr::fr_service::NewStub(channel)} {
  // Constructor body (if needed)
}

server_rpc_manager::~server_rpc_manager() {
  is_running.store(false);
  if (periodic_thread.joinable()) {
    periodic_thread.join();
  }
}

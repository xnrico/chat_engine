#include "server/rpc/server_rpc_manager.hpp"

// logging includes
#include <grpcpp/grpcpp.h>

#include <chrono>
#include <condition_variable>
#include <mutex>

#include "common/chat_utils.hpp"
#include "server/sessions/camera_receiver.hpp"

using namespace std::chrono_literals;

server_rpc_manager::server_rpc_manager()
    : channel{grpc::CreateChannel("localhost:6002", grpc::InsecureChannelCredentials())},
      stub{robot::robot_service::NewStub(channel)} {
  // Constructor body (if needed)
  is_running.store(true);
  periodic_thread = std::thread([this]() {
    while (is_running.load()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      cleanup_sessions();
    }
  });
}

server_rpc_manager::~server_rpc_manager() {
  is_running.store(false);
  if (periodic_thread.joinable()) {
    periodic_thread.join();
  }
}

grpc::Status server_rpc_manager::init_camera_stream(grpc::ServerContext* context,
                                                    const server::init_camera_offer* request,
                                                    server::init_camera_answer* response) {
  // Implementation of the method
  if (!request || request->sdp().empty()) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty offer SDP");
  }

  if (sessions.size() >= MAX_SESSIONS) {
    LOG_WARNING(logger, "Max sessions reached, cannot create new session");
    return grpc::Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "Max sessions reached");
  }

  const auto& session_id = request->session_id();

  // Create a new camera receiver
  std::lock_guard<std::mutex> lock(mtx);
  sessions.try_emplace(session_id, std::make_shared<camera_receiver>(session_id, stub));
  auto receiver = std::dynamic_pointer_cast<camera_receiver>(sessions[session_id]);

  auto answer_sdp = receiver->create_receiver(request->sdp());

  response->set_session_id(session_id);
  response->set_sdp(answer_sdp);

  return answer_sdp.empty() ? grpc::Status(grpc::StatusCode::INTERNAL, "Failed to create camera receiver")
                            : grpc::Status::OK;
}

void server_rpc_manager::cleanup_sessions() {
  std::lock_guard<std::mutex> lock(mtx);
  for (auto it = sessions.begin(); it != sessions.end();) {
    if (!it->second->is_active()) {
      LOG_INFO(logger, "Cleaning up inactive session: {}", it->first);
      it = sessions.erase(it);
    } else {
      ++it;
    }
  }
}
#include "client/rpc/robot_rpc_manager.hpp"

#include <grpcpp/grpcpp.h>

#include "client/sessions/camera_streamer.hpp"
#include "common/chat_utils.hpp"

robot_rpc_manager::robot_rpc_manager()
    : channel{grpc::CreateChannel("localhost:6001", grpc::InsecureChannelCredentials())},
      stub{server::server_service::NewStub(channel)} {
  // Constructor body (if needed)
  is_running.store(true);
  periodic_thread = std::thread([this]() {
    while (is_running.load()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      cleanup_sessions();
    }
  });
}

robot_rpc_manager::~robot_rpc_manager() {
  is_running.store(false);
  if (periodic_thread.joinable()) {
    periodic_thread.join();
  }
}

grpc::Status robot_rpc_manager::stop_camera_stream(grpc::ServerContext* context, const robot::generic_message* request,
                                                   robot::response_message* response) {
  // Implementation of the method
  response->set_session_id(request->session_id());
  response->set_success(true);
  return grpc::Status::OK;
}

grpc::Status robot_rpc_manager::offer(grpc::ServerContext* context, const robot::offer_request* request,
                                      robot::offer_response* response) {
  if (!request || request->sdp().empty() || request->session_id().empty()) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty offer SDP or session ID");
  }

  return grpc::Status::OK;
}

std::string robot_rpc_manager::init_camera_stream(
    std::function<void()> on_start = [] {}, std::function<void()> on_server_error = [] {},
    std::function<void()> on_camera_error = [] {}, std::function<void()> on_timeout = [] {},
    std::function<void()> on_end = [] {}) {
  // Implementation of the method
  auto sid = generate_id();
  std::lock_guard<std::mutex> lock(mtx);
  sessions.try_emplace(sid, std::make_shared<camera_streamer>(sid, 6000, stub));

  auto streamer = std::dynamic_pointer_cast<camera_streamer>(sessions[sid]);
  streamer->set_on_start(on_start);
  streamer->set_on_server_error(on_server_error);
  streamer->set_on_camera_error(on_camera_error);
  streamer->set_on_timeout(on_timeout);
  streamer->set_on_end(on_end);
  streamer->create_stream();

  return sid;
}

void robot_rpc_manager::stop_camera_stream(const std::string& session_id) {
  std::lock_guard<std::mutex> lock(mtx);
  auto it = sessions.find(session_id);
  if (it != sessions.end()) {
    std::dynamic_pointer_cast<camera_streamer>(it->second)->remove_stream();
  }
}

void robot_rpc_manager::cleanup_sessions() {
  std::lock_guard<std::mutex> lock(mtx);

  // Use an explicit iterator
  for (auto it = sessions.begin(); it != sessions.end();) {
    // Check if the session is inactive
    if (!it->second->is_active()) {
      LOG_INFO(logger, "Cleaning up inactive session: {}", it->first);
      it = sessions.erase(it);
    } else {
      // If the element is kept, move to the next element.
      ++it;
    }
  }
}
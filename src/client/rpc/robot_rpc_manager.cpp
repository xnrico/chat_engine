#include "client/rpc/robot_rpc_manager.hpp"

// logging includes
#include <grpcpp/grpcpp.h>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/Logger.h>
#include <quill/sinks/ConsoleSink.h>

#include "client/sessions/camera_streamer.hpp"
#include "common/chat_utils.hpp"

robot_rpc_manager::robot_rpc_manager()
    : logger{quill::Frontend::create_or_get_logger(
          getenv("USER") ? getenv("USER") : "unknown_user",
          quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_robot_rpc"))},
      channel{grpc::CreateChannel("localhost:6001", grpc::InsecureChannelCredentials())},
      stub{server::server_service::NewStub(channel)},
      on_stream_start{[]() {}},
      on_stream_end{[]() {}},
      on_stream_failed{[]() {}} {
  // Constructor body (if needed)
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

void robot_rpc_manager::init_camera_stream() {
  // Implementation of the method
  auto sid = generate_id();
  sessions.emplace(sid, std::make_shared<camera_streamer>(sid, 6000, stub));
  std::dynamic_pointer_cast<camera_streamer>(sessions[sid])->create_stream();
}
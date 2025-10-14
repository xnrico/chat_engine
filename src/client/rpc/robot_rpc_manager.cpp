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

void robot_rpc_manager::init_fr_request(const std::string& session_id, size_t timeout, std::function<void()> on_success,
                                        std::function<void()> on_failure, std::function<void()> on_timeout) {
  auto request = fr::fr_request_message{};
  request.set_session_id(session_id);

  auto context = grpc::ClientContext{};
  context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(timeout));

  auto response = fr::fr_response_message{};

  grpc::Status status = fr_stub->init_fr_request(&context, request, &response);  // block until response or timeout

  if (status.ok()) {
    // Handle successful response
    on_success();
  } else if (status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED) {
    // Handle error timeout
    on_timeout();
  } else {
    // Handle other errors
    on_failure();
  }
}

void robot_rpc_manager::cancel_fr_request(const std::string& session_id, size_t timeout,
                                          std::function<void()> on_success, std::function<void()> on_failure,
                                          std::function<void()> on_timeout) {
  auto request = fr::fr_request_message{};
  request.set_session_id(session_id);

  auto context = grpc::ClientContext{};
  context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(timeout));

  auto response = fr::fr_response_message{};

  grpc::Status status = fr_stub->cancel_fr_request(&context, request, &response);  // block until

  if (status.ok()) {
    // Handle successful response
    on_success();
  } else if (status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED) {
    // Handle error timeout
    on_timeout();
  } else {
    // Handle other errors
    on_failure();
  }
}
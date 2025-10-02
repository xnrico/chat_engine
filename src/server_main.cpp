#include <iostream>
#include <memory>
#include <tinyfsm/tinyfsm.hpp>

#include "common/chat_utils.hpp"
#include "server/rpc/server_rpc_manager.hpp"

int main(int argc, char* argv[]) {
  logger = std::shared_ptr<quill::Logger>{
      quill::Frontend::create_or_get_logger(getenv("USER") ? getenv("USER") : "unknown_user",
                                            quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_server"))};

  logger->set_log_level(quill::LogLevel::TraceL3);
  quill::Backend::start();

  auto server = server_rpc_manager{};

  grpc::ServerBuilder builder;
  builder.AddListeningPort("0.0.0.0:6001", grpc::InsecureServerCredentials());
  builder.RegisterService(&server);
  std::unique_ptr<grpc::Server> grpc_server = builder.BuildAndStart();

  std::cout << "Hello, Chat Engine!" << std::endl;
  grpc_server->Wait();
  std::cout << "Goodbye, Chat Engine!" << std::endl;

  return 0;
}
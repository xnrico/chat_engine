#include <iostream>
#include <memory>
#include <server/server_rpc_manager.hpp>
#include <tinyfsm/tinyfsm.hpp>

int main(int argc, char* argv[]) {
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
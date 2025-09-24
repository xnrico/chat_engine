#include <iostream>
#include <memory>
#include <server/chat_server.hpp>
#include <server/signaling_server.hpp>
#include <tinyfsm/tinyfsm.hpp>

int main(int argc, char* argv[]) {
  auto chat = chat_server{};
  auto signaling = signaling_server{};

  grpc::ServerBuilder builder;
  builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
  builder.RegisterService(&chat);
  builder.RegisterService(&signaling);
  std::unique_ptr<grpc::Server> grpc_server = builder.BuildAndStart();

  std::cout << "Hello, Chat Engine!" << std::endl;
  grpc_server->Wait();
  std::cout << "Goodbye, Chat Engine!" << std::endl;

  return 0;
}
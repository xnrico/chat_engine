#include <iostream>
#include <memory>
#include <server/chat_server.hpp>
#include <tinyfsm/tinyfsm.hpp>

int main(int argc, char* argv[]) {
  auto server = chat_server{};

  ServerBuilder builder;
  builder.AddListeningPort("0.0.0.0:50051", InsecureServerCredentials());
  builder.RegisterService(&server);
  std::unique_ptr<Server> grpc_server = builder.BuildAndStart();

  std::cout << "Hello, Chat Engine!" << std::endl;
  grpc_server->Wait();
  std::cout << "Goodbye, Chat Engine!" << std::endl;

  return 0;
}
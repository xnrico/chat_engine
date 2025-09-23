#include "client/states/client_state_manager.hpp"
#include "grpc/chat.grpc.pb.h"

#include <grpcpp/grpcpp.h>
#include <thread>
#include <chrono>

using namespace grpc;
using namespace chat;
using namespace std::chrono_literals;

int main(int argc, char* argv[]) {
  auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
  auto stub = chat::chat_service::NewStub(channel);
  auto& csm = client_state_manager::get_instance();

  std::cout << "Start of client\n";
  csm.start();

  // Main client loop
  while (true) {
    std::this_thread::sleep_for(100ms);
  }

  return 0;
}
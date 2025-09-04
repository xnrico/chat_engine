#include "client/chat_client.hpp"
#include "client/client_states.hpp"
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

  std::cout << "Start of client\n";
  robot::start();

  


  return 0;
}
#include "client/chat_client.hpp"
#include "grpc/chat.grpc.pb.h"

#include <grpcpp/grpcpp.h>

using namespace grpc;
using namespace chat;

int main(int argc, char* argv[]) {
  auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
  auto stub = chat::chat_service::NewStub(channel);

  // Example usage of the client
  // gRPC call to start the voice request
  start_voice_request_message request;
  request_id_message response;

  request.set_request_id("1");
  ClientContext context;

  Status status = stub->start_voice_request(&context, request, &response);

  if (status.ok()) {
    std::cout << "Client: Request successfully started with server for ID: " << response.request_id() << std::endl;

    // This is where WebRTC audio streaming would begin
    // The audio stream would be tied to the current request_id
  } else {
    std::cerr << "Client: StartVoiceRequest failed: " << status.error_message() << std::endl;
  }

  return 0;
}
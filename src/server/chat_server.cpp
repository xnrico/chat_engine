#include "server/chat_server.hpp"

#include <grpcpp/grpcpp.h>

#include <chrono>
#include <thread>

#include "grpc/chat.grpc.pb.h"

using namespace chat;
using namespace grpc;
using namespace std::chrono_literals;

Status chat_server::start_facial_recognition(ServerContext* context, const start_facial_recognition_message* request,
                                             request_id_message* response) {
  // Implement your facial recognition start logic here
  return Status::OK;
}

Status chat_server::start_voice_request(ServerContext* context, const start_voice_request_message* request,
                                        request_id_message* response) {
  // Implement your voice request start logic here
  std::cout << "Server: Received StartVoiceRequest for ID: " << request->request_id() << std::endl;

  // Simulate asynchronous processing
  // In a real application, this would launch a thread to handle STT, LLM, etc.
  std::thread([]() {
    // Simulate a delay for processing
    std::this_thread::sleep_for(3s);

    std::cout << "Server: Processing complete for " << 1 << ". Generating response." << std::endl;

    // Simulate TTS and streaming back to client via WebRTC...

    // For now, we'll just mark as finished
    std::cout << "Server: Sending response for " << 1 << "." << std::endl;
  }).detach();

  response->set_request_id(request->request_id());
  return Status::OK;
}

Status chat_server::cancel_voice_request(ServerContext* context, const request_id_message* request,
                                         empty_message* response) {
  // Implement your voice request cancellation logic here
  return Status::OK;
}
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

  std::cout << "\n--- Scenario 1: Normal Interaction ---" << std::endl;
  std::this_thread::sleep_for(2s);
  robot::dispatch(human_presence_event{true});
  std::this_thread::sleep_for(2s);
  robot::dispatch(user_speech_detected_event{true});
  robot::dispatch(facial_recognition_response_event{});
  std::this_thread::sleep_for(1s);
  robot::dispatch(webrtc_video_stopped_event{});
  std::this_thread::sleep_for(2s);
  robot::dispatch(webrtc_greeting_audio_finished_event{});
  robot::dispatch(user_speech_detected_event{false});
  std::this_thread::sleep_for(4s);
  robot::dispatch(user_speech_detected_event{true});
  std::this_thread::sleep_for(2s);
  robot::dispatch(user_speech_detected_event{false});
  


  // // Example usage of the client
  // // gRPC call to start the voice request
  // start_voice_request_message request;
  // request_id_message response;

  // request.set_request_id("1");
  // ClientContext context;

  // Status status = stub->start_voice_request(&context, request, &response);

  // if (status.ok()) {
  //   std::cout << "Client: Request successfully started with server for ID: " << response.request_id() << std::endl;

  //   // This is where WebRTC audio streaming would begin
  //   // The audio stream would be tied to the current request_id
  // } else {
  //   std::cerr << "Client: StartVoiceRequest failed: " << status.error_message() << std::endl;
  // }

  return 0;
}
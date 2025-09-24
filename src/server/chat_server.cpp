#include "server/chat_server.hpp"

#include <grpcpp/grpcpp.h>

#include <chrono>
#include <thread>

#include "grpc/chat.grpc.pb.h"

using namespace chat;
using namespace grpc;
using namespace std::chrono_literals;

chat_server::chat_server() {}
chat_server::~chat_server() {}

grpc::Status chat_server::init_camera_stream(grpc::ServerContext* context, const chat::generic_message* request,
                                             chat::response_message* response) {
  return grpc::Status::OK;
}

grpc::Status chat_server::stop_camera_stream(grpc::ServerContext* context, const chat::request_id_message* request,
                                             chat::response_message* response) {
  return grpc::Status::OK;
}
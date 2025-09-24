#pragma once

#include <grpc/chat.grpc.pb.h>
#include <grpcpp/grpcpp.h>

#include <memory>
#include <unordered_map>

class chat_server final : public chat::chat_service::Service {
 private:
  std::unordered_map<std::string, std::shared_ptr<chat::request_id_message>> active_requests_;

 public:
  chat_server();
  ~chat_server();

  grpc::Status init_camera_stream(grpc::ServerContext* context, const chat::generic_message* request,
                                  chat::response_message* response) override;

  grpc::Status stop_camera_stream(grpc::ServerContext* context, const chat::request_id_message* request,
                                  chat::response_message* response) override;
};
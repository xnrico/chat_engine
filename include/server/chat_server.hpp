#pragma once

#include <grpc/chat.grpc.pb.h>
#include <grpcpp/grpcpp.h>

#include <unordered_map>
#include <memory>

using namespace chat;
using namespace grpc;

class chat_server : public chat_service::Service {
 private:
  std::unordered_map<std::string, std::shared_ptr<request_id_message>> active_requests_;

 public:
  Status start_facial_recognition(ServerContext* context, const start_facial_recognition_message* request,
                                  request_id_message* response) override;
  Status start_voice_request(ServerContext* context, const start_voice_request_message* request,
                             request_id_message* response) override;

  Status cancel_voice_request(ServerContext* context, const request_id_message* request,
                              empty_message* response) override;
};
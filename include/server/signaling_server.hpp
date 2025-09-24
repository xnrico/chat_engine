#pragma once

#include <grpc/signaling.grpc.pb.h>
#include <grpcpp/grpcpp.h>

#include <memory>
#include <unordered_map>

class signaling_server final : public signaling::signaling_service::Service {
 public:
  signaling_server();
  ~signaling_server();

  grpc::Status offer(grpc::ServerContext* context, const signaling::offer_request* request,
                     signaling::offer_response* response) override;
};
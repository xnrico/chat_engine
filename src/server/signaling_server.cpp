#include "server/signaling_server.hpp"

#include <grpcpp/grpcpp.h>

#include <chrono>
#include <thread>

#include "grpc/signaling.grpc.pb.h"

using namespace std::chrono_literals;

signaling_server::signaling_server() {}
signaling_server::~signaling_server() {}

grpc::Status signaling_server::offer(grpc::ServerContext* context, const signaling::offer_request* request,
                                     signaling::offer_response* response) {
  // Handle the offer request
  response->set_sdp("This is a response SDP from the server.");
  return grpc::Status::OK;
}
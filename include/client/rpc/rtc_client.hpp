#pragma once

#include <grpcpp/grpcpp.h>
#include <quill/Logger.h>

#include <memory>
#include <rtc/rtc.hpp>

#include "grpc/chat.grpc.pb.h"
#include "grpc/signaling.grpc.pb.h"

class rtc_client final {
 private:
  std::unordered_map<std::string, std::shared_ptr<rtc::DataChannel>> data_channels;
  std::unordered_map<std::string, std::shared_ptr<rtc::PeerConnection>> peer_connections;

  std::string local_id;
  rtc::Configuration config;
  std::shared_ptr<quill::Logger> logger;

  std::shared_ptr<grpc::Channel> channel;
  std::unique_ptr<chat::chat_service::Stub> chat_stub;
  std::unique_ptr<signaling::signaling_service::Stub> signaling_stub;

  auto generate_id(size_t n) -> std::string;
  auto create_pc(const std::string& remote_id) -> std::shared_ptr<rtc::PeerConnection>;
  auto stream_media(int sock) -> void;

 public:
  rtc_client();
  ~rtc_client();
};
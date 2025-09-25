#pragma once

#include <grpcpp/grpcpp.h>
#include <quill/Logger.h>

#include <memory>
#include <rtc/rtc.hpp>

#include "grpc/chat.grpc.pb.h"
#include "grpc/signaling.grpc.pb.h"

class rtc_client final : public std::enable_shared_from_this<rtc_client> {
 private:
  struct connection_info {
    std::shared_ptr<rtc::PeerConnection> pc;
    int socket;
    bool active;
  };

  std::unordered_map<std::string, connection_info> peer_connections;

  std::string local_id;
  rtc::Configuration config;
  std::shared_ptr<quill::Logger> logger;

  std::shared_ptr<grpc::Channel> channel;
  std::unique_ptr<chat::chat_service::Stub> chat_stub;
  std::unique_ptr<signaling::signaling_service::Stub> signaling_stub;

  // Callbacks
  std::function<void()> on_stream_start;
  std::function<void()> on_stream_end;
  std::function<void()> on_stream_failed;

  auto generate_id(size_t n) -> std::string;
  auto create_pc(const std::string& remote_id) -> std::shared_ptr<rtc::PeerConnection>;
  auto stream_media(int sock) -> void;
  auto close_pc(const std::string& remote_id) -> void;
  auto cleanup_pc() -> void;

 public:
  rtc_client();
  ~rtc_client();

  auto set_on_stream_start(std::function<void()>&& callback) noexcept -> void;
  auto set_on_stream_end(std::function<void()>&& callback) noexcept -> void;
  auto set_on_stream_failed(std::function<void()>&& callback) noexcept -> void;

  auto start_camera_stream() -> void;
};
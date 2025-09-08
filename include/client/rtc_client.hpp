#pragma once

#include <rtc/rtc.hpp>
#include <memory>

class rtc_client final {
  private:
    std::string local_id;
    rtc::Configuration config;
    std::unordered_map<std::string, std::shared_ptr<rtc::DataChannel>> data_channels;
    std::unordered_map<std::string, std::shared_ptr<rtc::PeerConnection>> peer_connections;

    auto generate_id() -> std::string;
    auto create_pc(std::weak_ptr<rtc::WebSocket> wws, const std::string& remote_id) -> std::shared_ptr<rtc::PeerConnection>;

  public:
    rtc_client();
    ~rtc_client();
};
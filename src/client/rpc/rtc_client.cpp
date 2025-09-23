#include "client/rpc/rtc_client.hpp"

#include <nlohmann/json.hpp>
#include <random>

rtc_client::rtc_client() {
  // Initialize WebSocket and PeerConnection
}

rtc_client::~rtc_client() {
  // Clean up resources
}

auto generate_id(size_t n) -> std::string {  // generate random id of length n
  static thread_local std::mt19937 rng(
      static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
  static const std::string dictionary("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

  auto id = std::string(n, '0');
  auto uniform = std::uniform_int_distribution<int>(0, int(dictionary.size() - 1));
  std::generate(id.begin(), id.end(), [&]() { return dictionary.at(uniform(rng)); });
  return id;
}

auto rtc_client::create_pc(std::weak_ptr<rtc::WebSocket> wws, const std::string& remote_id)
    -> std::shared_ptr<rtc::PeerConnection> {
  auto pc = std::make_shared<rtc::PeerConnection>(config);

  pc->onStateChange([](rtc::PeerConnection::State state) {
    std::cout << "PeerConnection state changed: " << static_cast<int>(state) << std::endl;
  });

  pc->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
    std::cout << "Gathering state changed: " << static_cast<int>(state) << std::endl;
  });

  pc->onLocalDescription([wws, remote_id](rtc::Description description) {
    nlohmann::json message = {
        {"id", remote_id}, {"type", description.typeString()}, {"description", std::string(description)}};

    if (auto ws = wws.lock()) ws->send(message.dump());
  });

  pc->onDataChannel([this, remote_id](std::shared_ptr<rtc::DataChannel> dc) {
    std::cout << "DataChannel from " << remote_id << " received with label \"" << dc->label() << "\"" << std::endl;

    dc->onOpen([this, wdc = std::weak_ptr(dc)]() {
      if (auto dc = wdc.lock()) dc->send("Hello from " + local_id);
    });

    dc->onClosed([remote_id]() { std::cout << "DataChannel from " << remote_id << " closed" << std::endl; });

    dc->onMessage([remote_id](auto data) {
      // data holds either std::string or rtc::binary
      if (std::holds_alternative<std::string>(data))
        std::cout << "Message from " << remote_id << " received: " << std::get<std::string>(data) << std::endl;
      else
        std::cout << "Binary message from " << remote_id << " received, size=" << std::get<rtc::binary>(data).size()
                  << std::endl;
    });

    data_channels.emplace(remote_id, dc);
  });

  peer_connections.emplace(remote_id, pc);
  return pc;
}
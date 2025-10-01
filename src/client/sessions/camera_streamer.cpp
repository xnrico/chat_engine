#include "client/sessions/camera_streamer.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <future>

std::unordered_map<int, camera_streamer::capture> camera_streamer::captures;

// Helper functions
int make_socket(int port, size_t buffer_size) {
  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(port);

  if (bind(sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0) {
    ::close(sock);
    throw std::runtime_error("Failed to bind UDP socket on 127.0.0.1:" + std::to_string(port));
  }

  setsockopt(sock, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&buffer_size), sizeof(buffer_size));
  return sock;
}

// Class methods

void camera_streamer::capture_work(int port) {
  std::array<char, 2048> buffer;
  auto sock = captures[port].socket;

  while (captures[port].is_running.load()) {
    int len;
    while ((len = recv(sock, buffer.data(), buffer.size(), 0)) >= 0) {
      if (len < sizeof(rtc::RtpHeader)) continue;  // Ignore invalid packets
      // Process RTP packet if needed
      // Dispatch to all uplinks with synchronization barrier
      std::vector<std::future<void>> futures;
      futures.reserve(captures[port].uplinks.size());
      
      // Asynchronously send to all uplinks
      for (const auto& link : captures[port].uplinks) {
        futures.emplace_back(std::async(std::launch::async, [&link, &buffer, len]() { link.on_data(buffer, len); }));
      }

      // Wait for all threads to complete before updating the buffer again
      for (auto& future : futures) {
        future.wait();
      }
    }
  }
}

void camera_streamer::dispatch_uplink(const uplink& link) {
  if (captures.find(rtp_port) == captures.end()) {
    // First uplink for this port, start capture
    captures.try_emplace(rtp_port);
    captures[rtp_port].uplinks.push_back(link);
    captures[rtp_port].is_running.store(true);
    captures[rtp_port].socket = make_socket(rtp_port, BUFFER_SIZE);
    captures[rtp_port].capture_thread =
        std::thread([this]() { this->capture_work(rtp_port); });  // Start capture thread for the port
  } else {
    // Add uplink to existing capture
    captures[rtp_port].uplinks.push_back(link);
  }
}

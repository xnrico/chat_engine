#include "client/sessions/camera_streamer.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <future>

#include "common/chat_utils.hpp"

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

  struct timeval timeout;
  timeout.tv_sec = 1;  // 1 second timeout for recv()
  timeout.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  setsockopt(sock, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&buffer_size), sizeof(buffer_size));

  return sock;
}

// Class methods

void camera_streamer::capture_work(int port) {
  std::array<char, 2048> buffer;
  auto sock = captures[port].socket;
  captures[port].has_data.store(false);
  int len{};

  LOG_DEBUG(logger, "Waiting for RTP packets on port {}", port);

  while (captures[port].is_running.load()) {
    // Try to read
    len = recv(sock, buffer.data(), buffer.size(), 0);  // returns -1 when no data

    if (len < 0 || len < sizeof(rtc::RtpHeader)) {
      captures[port].has_data.store(false);
      captures[port].capture_cv.notify_all();
      LOG_DEBUG(logger, "Invalid RTP packet received on port {}, Number of links {}", port,
                captures[port].uplinks.size());
      for (const auto& link : captures[port].uplinks) link.on_camera_error();
      std::this_thread::sleep_for(10ms);  // Wait before retrying
      continue;                           // Ignore invalid packets
    }

    LOG_DEBUG(logger, "Received RTP packet of size {} on port {}", len, port);

    // Process RTP packet if needed
    // Dispatch to all uplinks with synchronization barrier
    std::vector<std::future<void>> futures;
    futures.reserve(captures[port].uplinks.size());
    captures[port].has_data.store(true);
    captures[port].capture_cv.notify_all();

    // Asynchronously send to all uplinks
    for (const auto& link : captures[port].uplinks) {
      futures.emplace_back(std::async(std::launch::async, [&link, &buffer, len]() { link.on_data(buffer, len); }));
    }

    // Wait for all threads to complete before updating the buffer again
    for (auto& future : futures) {
      future.wait();
    }
  }

  captures[port].has_data.store(false);
  captures[port].capture_cv.notify_all();

  LOG_DEBUG(logger, "Stopping RTP capture on port {}", port);
}

void camera_streamer::dispatch_uplink(const uplink& link) {
  if (captures.find(rtp_port) == captures.end()) {
    // First uplink for this port, start capture
    captures.try_emplace(rtp_port);
    captures[rtp_port].is_running.store(true);
    captures[rtp_port].socket = make_socket(rtp_port, BUFFER_SIZE);
    captures[rtp_port].capture_thread =
        std::thread([this]() { this->capture_work(rtp_port); });  // Start capture thread for the port
  }

  // Use a separate scope for the lock to avoid deadlock
  {
    std::unique_lock<std::mutex> lock(captures[rtp_port].capture_mtx);
    captures[rtp_port].capture_cv.wait_for(lock, std::chrono::seconds(5),
                                           [this]() { return captures[rtp_port].has_data.load(); });
  }

  // Add uplink to existing capture
  if (captures[rtp_port].has_data.load()) {
    captures[rtp_port].uplinks.push_back(link);
    link.on_start();  // Notify uplink that streaming has started
  } else {
    link.on_timeout();  // Notify uplink of timeout
  }
}

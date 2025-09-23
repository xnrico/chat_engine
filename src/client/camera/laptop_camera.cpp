#include "client/camera/laptop_camera.hpp"

#include <chrono>

using namespace std::chrono_literals;

laptop_camera::laptop_camera() {}

auto laptop_camera::detect_objects() -> void {
  while (is_running.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));  // Capture is only at 60 FPS
  }
}

auto laptop_camera::process_objects() -> void {
  static auto last_time = std::chrono::steady_clock::now();
  static auto human_present = false;

  // Simulate human detection
  if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - last_time) > 1s) {
    last_time = std::chrono::steady_clock::now();
    human_present = rand() % 2 == 0;
  }

  // Invokes callbacks
  if (human_detected.exchange(human_present) != human_detected.load()) {
    if (human_detected.load()) {
      on_human_detected();
    } else {
      on_human_lost();
    }
  }
}

auto laptop_camera::start() -> bool {
  is_running.store(true);
  video_capture.store(true);
  object_detection.store(true);

  detection_thread = std::thread([this]() { this->detect_objects(); });
  return true;
}

auto laptop_camera::stop() -> void {
  object_detection.store(false);
  video_capture.store(false);
  is_running.store(false);

  if (detection_thread.joinable()) detection_thread.join();
}
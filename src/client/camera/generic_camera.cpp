#include "client/camera/generic_camera.hpp"
#include <chrono>

generic_camera::generic_camera()
    : is_running{false}, human_detected{false}, video_capture{false}, object_detection{false} {}

auto generic_camera::start() -> bool {
  if (is_running) return true;
  
  is_running = true;
  detection_thread = std::thread([this]() {
    while (is_running) {
      if (object_detection) {
        detect_objects();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });
  
  return true;
}

auto generic_camera::stop() -> void {
  if (!is_running) return;
  
  is_running = false;
  if (detection_thread.joinable()) {
    detection_thread.join();
  }
}
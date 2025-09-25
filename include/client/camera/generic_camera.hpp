#pragma once

#include <atomic>
#include <functional>
#include <iostream>
#include <thread>

#include "base_camera.hpp"

class generic_camera : public base_camera {
 protected:
  // Periodically checks for object detection
  std::thread detection_thread;

  // callback functions
  std::function<void()> on_human_detected;
  std::function<void()> on_human_lost;

  std::atomic<bool> is_running;
  std::atomic<bool> human_detected;
  std::atomic<bool> video_capture;
  std::atomic<bool> object_detection;

  virtual auto detect_objects() -> void {}

 public:
  generic_camera();
  virtual ~generic_camera() = default;

  virtual auto start() -> bool;
  virtual auto stop() -> void;

  virtual auto set_on_human_detected(std::function<void()>&& callback) noexcept -> void {
    on_human_detected = std::move(callback);
  }

  virtual auto set_on_human_lost(std::function<void()>&& callback) noexcept -> void {
    on_human_lost = std::move(callback);
  }
};
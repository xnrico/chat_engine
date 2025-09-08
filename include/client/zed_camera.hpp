#pragma once

#include <atomic>
#include <fstream>
#include <functional>
#include <iostream>
#include <sl/Camera.hpp>
#include <thread>

class zed_camera final {
 private:
  sl::Camera camera;
  sl::InitParameters init_params;
  sl::StreamingParameters stream_params;
  sl::ObjectDetectionParameters obj_params;
  sl::ObjectDetectionRuntimeParameters runtime_params;

  sl::Mat image_left;
  sl::Mat image_right;
  sl::Objects objects;

  std::atomic<bool> is_running;
  std::atomic<bool> human_detected;
  std::atomic<bool> video_capture;
  std::atomic<bool> object_detection;

  // Periodically checks for object detection
  std::thread detection_thread;

  // callback functions
  std::function<void()> on_human_detected;
  std::function<void()> on_human_lost;

  auto detect_objects() -> void;
  auto process_objects() -> void;

 public:
  zed_camera();
  ~zed_camera() { stop(); }

  auto start() -> bool;
  auto stop() -> void;

  auto set_on_human_detected(std::function<void()> callback) noexcept -> void {
    on_human_detected = std::move(callback);
  }
  auto set_on_human_lost(std::function<void()> callback) noexcept -> void { on_human_lost = std::move(callback); }
};
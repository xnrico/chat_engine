#pragma once

#include <atomic>
#include <fstream>
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
  std::thread detection_thread;

  auto detect_objects() -> void;
  auto process_objects() -> void;

 public:
  zed_camera();
  ~zed_camera() { stop(); }

  auto start() -> bool;
  auto stop() -> void;
  
  auto is_human_detected() const -> bool { return human_detected.load(); }
};
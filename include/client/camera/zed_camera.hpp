#pragma once

#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <sl/Camera.hpp>
#include <thread>

#include "generic_camera.hpp"

class zed_camera final : public generic_camera {
 private:
  const int SN = 30477778;

  sl::Camera camera;
  sl::InitParameters init_params;
  sl::StreamingParameters stream_params;
  sl::ObjectDetectionParameters obj_params;
  sl::ObjectDetectionRuntimeParameters runtime_params;

  sl::Mat image_left;
  sl::Mat image_right;
  sl::Objects objects;

  // Filtering parameters
  static constexpr size_t DETECTION_WINDOW_SIZE = 60;                   // Number of frames to consider
  static constexpr size_t MIN_DETECTIONS_FOR_ON = 50;                   // Require 50/60 frames to trigger detection
  static constexpr size_t MIN_DETECTIONS_FOR_OFF = 50;                  // Require 50/60 frames to clear detection
  static constexpr float CONFIDENCE_THRESHOLD_HIGH = 90.0f;             // Confidence threshold for detection
  static constexpr float CONFIDENCE_THRESHOLD_LOW = 85.0f;              // Hysteresis threshold
  static constexpr std::chrono::milliseconds MIN_STATE_DURATION{1000};  // Min time between state changes

  // Filtering state
  std::deque<bool> detection_history;
  std::chrono::steady_clock::time_point last_state_change;
  bool stable_human_detected = false;

  auto detect_objects() -> void override;

 private:
  auto process_objects() -> void;

 public:
  zed_camera();
  virtual ~zed_camera() override { stop(); }

  auto start() -> bool override;
  auto stop() -> void override;
};
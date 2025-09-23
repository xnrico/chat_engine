#pragma once

#include <atomic>
#include <functional>
#include <sl/Camera.hpp>
#include <thread>

#include "generic_camera.hpp"

class zed_camera final : public generic_camera {
 private:
  sl::Camera camera;
  sl::InitParameters init_params;
  sl::StreamingParameters stream_params;
  sl::ObjectDetectionParameters obj_params;
  sl::ObjectDetectionRuntimeParameters runtime_params;

  sl::Mat image_left;
  sl::Mat image_right;
  sl::Objects objects;

  auto detect_objects() -> void override;

 private:
  auto process_objects() -> void;

 public:
  zed_camera();
  virtual ~zed_camera() { stop(); }

  auto start() -> bool override;
  auto stop() -> void override;
};
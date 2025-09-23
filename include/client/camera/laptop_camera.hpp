#pragma once

#include <atomic>
#include <functional>
#include <iostream>
#include <thread>

#include "generic_camera.hpp"

class laptop_camera final : public generic_camera {
 protected:
  auto detect_objects() -> void override;

 private:
  std::atomic<bool> human_detected{false};
  auto process_objects() -> void;

 public:
  laptop_camera();
  virtual ~laptop_camera() { stop(); }

  auto start() -> bool override;
  auto stop() -> void override;
};
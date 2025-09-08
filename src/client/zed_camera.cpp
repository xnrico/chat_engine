#include "client/zed_camera.hpp"

#include <atomic>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <sl/Camera.hpp>
#include <thread>

auto zed_camera::detect_objects() -> void {
  auto ret = camera.enableObjectDetection(obj_params);

  if (ret != sl::ERROR_CODE::SUCCESS) {
    std::cerr << "Error enabling object detection: " << ret << std::endl;
    camera.close();
    return;
  }

  camera.setObjectDetectionRuntimeParameters(runtime_params);
  // Camera successfully enabled object detection with runtime parameters

  camera.enableStreaming(stream_params);

  while (is_running.load()) {
    if (camera.grab() == sl::ERROR_CODE::SUCCESS) {
      if (!video_capture.load() || !object_detection.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        continue;
      }

      camera.retrieveObjects(objects);
      process_objects();
    } else {
      std::cerr << "Error grabbing frame." << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(20));  // Capture is only at 60FPS
  }

  // Disable streaming and object detection
  camera.disableStreaming();
  camera.disableObjectDetection();
}

auto zed_camera::process_objects() -> void {
  auto human_present = false;

  for (const auto& obj : objects.object_list) {
    if (obj.label == sl::OBJECT_CLASS::PERSON && obj.confidence > 0.7) {
      human_present = true;
      break;
    }
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

zed_camera::zed_camera()
    : is_running{false},
      human_detected{false},
      video_capture{false},
      object_detection{false},
      on_human_detected{[]() {}},  // initialize to empty functions
      on_human_lost{[]() {}} {
  init_params.camera_resolution = sl::RESOLUTION::HD720;
  init_params.camera_fps = 60;
  init_params.depth_mode = sl::DEPTH_MODE::NEURAL;
  init_params.coordinate_units = sl::UNIT::METER;

  stream_params.codec = sl::STREAMING_CODEC::H265;  // H264 or H265
  stream_params.bitrate = 6000;                     // in Kbps, good for HD720 @ 60 FPS
  stream_params.port = 30000;                       // default port

  obj_params.enable_tracking = true;
  obj_params.enable_segmentation = true;

  runtime_params.detection_confidence_threshold = 0.5;
}

auto zed_camera::start() -> bool {
  auto ret = camera.open(init_params);
  if (ret != sl::ERROR_CODE::SUCCESS) {
    std::cerr << "Error opening camera: " << ret << std::endl;
    return false;
  }

  if (obj_params.enable_tracking) camera.enablePositionalTracking();  // benefits from higher FPS

  is_running.store(true);
  video_capture.store(true);
  object_detection.store(true);

  detection_thread = std::thread([this]() { this->detect_objects(); });

  return true;
}

auto zed_camera::stop() -> void {
  object_detection.store(false);
  video_capture.store(false);
  is_running.store(false);

  if (detection_thread.joinable()) detection_thread.join();
  camera.close();
}

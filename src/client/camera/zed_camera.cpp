#include "client/camera/zed_camera.hpp"

#include <atomic>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <sl/Camera.hpp>
#include <thread>

#include "common/chat_utils.hpp"

auto zed_camera::detect_objects() -> void {
  auto ret = camera.enableObjectDetection(obj_params);

  if (ret != sl::ERROR_CODE::SUCCESS) {
    LOG_ERROR(logger, "Error enabling object detection");
    camera.close();
    return;
  }

  ret = camera.setObjectDetectionRuntimeParameters(runtime_params);
  // Camera successfully enabled object detection with runtime parameters

  ret = camera.enableStreaming(stream_params);  // this would start new threads, if not received, there will be frequent
                                                // thread creations and exits

  while (is_running.load()) {
    if (camera.grab() == sl::ERROR_CODE::SUCCESS) {
      if (!video_capture.load() || !object_detection.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        continue;
      }

      camera.retrieveObjects(objects);
      process_objects();
    } else {
      LOG_ERROR(logger, "Error grabbing frame");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(20));  // Capture is only at 60 FPS
  }

  // Disable streaming and object detection
  camera.disableStreaming();
  camera.disableObjectDetection();
}

auto zed_camera::process_objects() -> void {
  auto human_present_in_frame = false;
  auto max_confidence = 0.0f;

  // Check if any person is detected in the current frame
  for (const auto &obj : objects.object_list) {
    if (obj.label == sl::OBJECT_CLASS::PERSON) {
      max_confidence = std::max(max_confidence, obj.confidence);

      // Use hysteresis: higher threshold when transitioning to detected,
      // lower threshold when already detected
      float threshold = stable_human_detected ? CONFIDENCE_THRESHOLD_LOW : CONFIDENCE_THRESHOLD_HIGH;

      if (obj.confidence > threshold) {
        human_present_in_frame = true;
        break;
      }
    }
  }

  // Add current detection to history
  detection_history.push_back(human_present_in_frame);

  // Keep history window at fixed size
  if (detection_history.size() > DETECTION_WINDOW_SIZE) {
    detection_history.pop_front();
  }

  // Count detections in the window
  size_t detection_count = 0;
  for (bool detected : detection_history) {
    if (detected) detection_count++;
  }

  // Only change state if we have enough history
  if (detection_history.size() == DETECTION_WINDOW_SIZE) {
    bool should_be_detected = false;

    // Apply different thresholds based on current state (hysteresis)
    if (stable_human_detected) {
      // Currently detected: require fewer detections to stay in detected state
      should_be_detected = (detection_count >= MIN_DETECTIONS_FOR_OFF);
    } else {
      // Currently not detected: require more detections to enter detected state
      should_be_detected = (detection_count >= MIN_DETECTIONS_FOR_ON);
    }

    // Check debounce timer
    auto now = std::chrono::steady_clock::now();
    auto time_since_last_change = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_state_change);

    // Only trigger callback if state changes and debounce period has elapsed
    if (should_be_detected != stable_human_detected && time_since_last_change >= MIN_STATE_DURATION) {
      stable_human_detected = should_be_detected;
      human_detected.store(stable_human_detected);
      last_state_change = now;

      if (stable_human_detected) {
        LOG_DEBUG(logger, "Human detected (confidence: {:.1f}%, {}/{} frames)", max_confidence, detection_count,
                  DETECTION_WINDOW_SIZE);
        on_human_detected();
      } else {
        LOG_DEBUG(logger, "Human lost ({}/{} frames)", detection_count, DETECTION_WINDOW_SIZE);
      }
    }
  }
}

zed_camera::zed_camera() {
  init_params.camera_resolution = sl::RESOLUTION::HD720;
  init_params.camera_fps = 60;
  init_params.depth_mode = sl::DEPTH_MODE::NEURAL;
  init_params.coordinate_units = sl::UNIT::METER;

  stream_params.codec = sl::STREAMING_CODEC::H264;  // H264 or H265
  stream_params.bitrate = 7000;                     // in Kbps, good for HD720 @ 60 FPS
  stream_params.port = 30000;                       // default port

  obj_params.enable_tracking = true;
  obj_params.enable_segmentation = true;

  runtime_params.detection_confidence_threshold = 0.5;

  // Initialize filtering state
  detection_history.clear();
  last_state_change = std::chrono::steady_clock::now();
  stable_human_detected = false;
}

auto zed_camera::start() -> bool {
  auto ret = camera.open(init_params);
  camera.reboot(zed_camera::SN, true);

  if (ret != sl::ERROR_CODE::SUCCESS) {
    LOG_ERROR(logger, "Error opening camera");
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
  LOG_DEBUG(logger, "ZED Camera closed");
}

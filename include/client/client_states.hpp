#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "tinyfsm/tinyfsm.hpp"

using namespace std::chrono_literals;

// Forward declarations

struct robot;
struct idle_state;
struct facial_recognition_state;
struct greeting_state;
struct waiting_for_speech_state;
struct streaming_speech_state;

// Special events
// Represents a change in human presence status
struct human_presence_event : tinyfsm::Event {
  bool present;
  human_presence_event(bool p) : present(p) {}
};

struct user_speech_detected_event : tinyfsm::Event {
  bool detected;
  user_speech_detected_event(bool d) : detected(d) {}
};

// Events for state transitions
struct generic_event : tinyfsm::Event {};
struct facial_recognition_response_event : tinyfsm::Event {};
struct webrtc_video_stopped_event : tinyfsm::Event {};
struct webrtc_greeting_audio_finished_event : tinyfsm::Event {};
struct webrtc_response_audio_finished_event : tinyfsm::Event {};
struct timeout_event : tinyfsm::Event {};
struct network_error_event : tinyfsm::Event {};

// Main state machine for robot (chat client)
struct robot : public tinyfsm::MealyMachine<robot> {
 public:
  virtual void react(const generic_event& e) {
    // Default event handler
    (void)e;
    std::cout << "Robot cannot handle event" << std::endl;
  }

  virtual void react(const human_presence_event&) { std::cout << "Robot cannot handle event" << std::endl; }
  virtual void react(const user_speech_detected_event&) { std::cout << "Robot cannot handle event" << std::endl; }
  virtual void react(const facial_recognition_response_event&) { std::cout << "Robot cannot handle event" << std::endl; }
  virtual void react(const webrtc_video_stopped_event&) { std::cout << "Robot cannot handle event" << std::endl; }
  virtual void react(const webrtc_greeting_audio_finished_event&) { std::cout << "Robot cannot handle event" << std::endl; }
  virtual void react(const webrtc_response_audio_finished_event&) { std::cout << "Robot cannot handle event" << std::endl; }
  virtual void react(const timeout_event&) { std::cout << "Robot cannot handle event" << std::endl; }
  virtual void react(const network_error_event&) { std::cout << "Robot cannot handle event" << std::endl; }

  virtual void entry() = 0;  // Entry action needs to be defined for all states
  virtual void exit() = 0; // Exit action needs to be defined for all states
};

// States
struct idle_state final : robot {
  void entry() override { std::cout << "[idle::entry] Entering idle state" << std::endl; }
  void exit() override { std::cout << "[idle::exit] Exiting idle state" << std::endl; }

  void react(const generic_event& e) override {
    std::cout << "[idle::react] Idle state ignoring generic event" << std::endl;
  }

  void react(const human_presence_event& e) override {
    transit<facial_recognition_state>(
        [&e]() -> void {
          // Action function
          std::cout << "[idle::react] Human present, transitioning to active state" << std::endl;
        },
        [&e]() -> bool {
          // Condition function
          return e.present;
        });
  }
};

struct facial_recognition_state final : robot {
  void entry() override {
    std::cout << "[facial_recognition::entry] gRPC call initiated. Waiting for response." << std::endl;
    // Simulate starting WebRTC video stream to server
    std::cout << "Starting WebRTC video stream to server..." << std::endl;
  }

  void exit() override {
    // Stop the video stream when leaving this state
    std::cout << "[facial_recognition::exit] Stopping WebRTC video stream." << std::endl;
  }

  void react(const facial_recognition_response_event& e) override {
    transit<greeting_state>(
        [&e]() -> void {
          // Action function
          std::cout << "[facial_recognition::react] Facial recognition response received." << std::endl;
        },
        [&e]() -> bool {
          // Condition function
          return true;  // Always transition for simplicity
        });
  }

  void react(const network_error_event& e) override {
    transit<idle_state>(
        [&e]() -> void {
          // Action function
          std::cout << "[facial_recognition::react] Network error occurred." << std::endl;
        },
        [&e]() -> bool {
          // Condition function
          return true;  // Always transition for simplicity
        });
  }
};

struct greeting_state final : robot {
  void entry() override {
    std::cout << "[greeting::entry] Starting WebRTC audio stream from server (TTS)." << std::endl;
    std::cout << "Playing greeting message..." << std::endl;
    std::this_thread::sleep_for(2s);
  }

  void exit() override { std::cout << "[greeting::exit] Exiting greeting state" << std::endl; }

  void react(const webrtc_greeting_audio_finished_event& e) override {
    transit<waiting_for_speech_state>(
        [&e]() -> void {
          // Action function
          std::cout << "[greeting::react] WebRTC greeting audio finished, transitioning to waiting for speech state"
                    << std::endl;
        },
        [&e]() -> bool {
          // Condition function
          return true;  // Always transition for simplicity
        });
  }
};

struct waiting_for_speech_state final : robot {
  void entry() override { std::cout << "[waiting_for_speech::entry] Waiting for user speech..." << std::endl; }

  void exit() override { std::cout << "[waiting_for_speech::exit] Exiting waiting for speech state" << std::endl; }

  void react(const user_speech_detected_event& e) override {
    transit<streaming_speech_state>(
        [&e]() -> void {
          // Action function
          std::cout << "[waiting_for_speech::react] User speech detected, transitioning to streaming speech state"
                    << std::endl;
        },
        [&e]() -> bool {
          // Condition function
          return e.detected;
        });
  }
};

struct streaming_speech_state final : robot {
  void entry() override {
    std::cout << "[streaming_speech::entry] Starting WebRTC audio stream from client..." << std::endl;
  }

  void exit() override { std::cout << "[streaming_speech::exit] Stopping WebRTC audio stream." << std::endl; }

  void react(const user_speech_detected_event& e) override {
    transit<idle_state>(
        [&e]() -> void {
          // Action function
          std::cout << "[streaming_speech::react] User speech finished, finished streaming and transitioning to idle state" << std::endl;
        },
        [&e]() -> bool {
          // Condition function
          return !e.detected;
        });
  }
};

FSM_INITIAL_STATE(robot, idle_state);
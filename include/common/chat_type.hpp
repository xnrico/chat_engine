#pragma once

#include <atomic>
#include <string>

struct request_type {
  std::string request_id;
  std::atomic<bool> is_processed;
  std::atomic<bool> is_canceled;

  request_type() : is_processed{false}, is_canceled{false} {}
  virtual ~request_type() = default;
};

struct voice_request_type final : public request_type {
 public:
  voice_request_type() : request_type{} {
    // Initialize voice request specific parameters
  }

  // Add voice request specific methods and members
};

struct facial_recognition_request_type final : public request_type {
 public:
  facial_recognition_request_type() : request_type{} {
    // Initialize facial recognition request specific parameters
  }

  // Add facial recognition request specific methods and members
};

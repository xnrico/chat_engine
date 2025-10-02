#pragma once

#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/Logger.h>
#include <quill/sinks/ConsoleSink.h>

#include <chrono>
#include <memory>
#include <random>
#include <string>

// Global logger instance shared across translation units
inline std::shared_ptr<quill::Logger> logger{};

// Function to generate a string of unique session ID
inline auto generate_id() -> std::string {
  static thread_local std::mt19937 rng(
      static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
  static const std::string dict = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::uniform_int_distribution<int> dist(0, static_cast<int>(dict.size() - 1));
  std::string out(8, '0');
  for (auto& c : out) c = dict[dist(rng)];
  return out;
}

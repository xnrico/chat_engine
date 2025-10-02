#pragma once
#include <string>
#include <atomic>

class base_session {
 protected:
  // Session ID
  std::string session_id;
  std::atomic<bool> session_active; // mark inactive for cleanup

 public:
  base_session(const std::string& sid) : session_id{sid}, session_active{true} {}
  virtual ~base_session() = default;

  std::string get_id() { return session_id; }
  bool is_active() const { return session_active.load(); }
};
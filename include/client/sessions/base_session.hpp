#pragma once
#include <string>

class base_session {
 protected:
  // Session ID
  std::string session_id;

 public:
  base_session(const std::string& sid) : session_id{sid} {}
  virtual ~base_session() = default;

  std::string get_id() { return session_id; }
};
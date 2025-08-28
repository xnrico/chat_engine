#pragma once

#include <tinyfsm/tinyfsm.hpp>

// Event declarations
struct speech_start : tinyfsm::Event {};
struct speech_end : tinyfsm::Event {};

// FSM Base class declaration

class engine : public tinyfsm::Fsm<engine> {
 private:
  friend class tinyfsm::Fsm<engine>;
  // Allow FSM to access private members
  virtual auto entry() -> void = 0; /* entry actions in some states */
  virtual auto exit() -> void = 0;  /* exit actions in some states */

  auto react(const tinyfsm::Event &e) -> void; // default reaction
  auto react(const speech_start &e) -> void;
  auto react(const speech_end &e) -> void;

 protected:
 public:
};
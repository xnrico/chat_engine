#include "client/client_state_manager.hpp"

client_state_manager::~client_state_manager() {
  // Clean up resources if needed
  if (camera) {
    camera->stop();
  }
}

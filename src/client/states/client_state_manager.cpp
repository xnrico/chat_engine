#include "client/states/client_state_manager.hpp"

#include "client/camera/zed_camera.hpp"
#include "client/rpc/robot_rpc_manager.hpp"

client_state_manager::client_state_manager()
    : camera{std::make_shared<zed_camera>()}, rpc_manager{std::make_shared<robot_rpc_manager>()} {
  bot::camera = camera;
  bot::rpc_manager = rpc_manager;
}

client_state_manager::~client_state_manager() {
  // Clean up resources if needed
  if (camera) {
    camera->stop();
  }
}

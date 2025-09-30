#include "client/states/client_states.hpp"

std::shared_ptr<generic_camera> bot::camera = nullptr;
std::shared_ptr<quill::Logger> bot::logger = nullptr;
std::shared_ptr<robot_rpc_manager> bot::rpc_manager = nullptr;

// Define the initial state here to avoid multiple definitions
FSM_INITIAL_STATE(bot, init_state);



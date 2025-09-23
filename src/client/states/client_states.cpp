#include "client/states/client_states.hpp"

std::shared_ptr<generic_camera> robot::camera = nullptr;
std::shared_ptr<rtc_client> robot::client = nullptr;

// Define the initial state here to avoid multiple definitions
FSM_INITIAL_STATE(robot, init_state);



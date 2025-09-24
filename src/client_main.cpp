#include "client/states/client_state_manager.hpp"

#include <thread>
#include <chrono>

using namespace grpc;
using namespace chat;
using namespace std::chrono_literals;

int main(int argc, char* argv[]) {
  auto& csm = client_state_manager::get_instance();

  std::cout << "Start of client\n";
  csm.start();

  // Main client loop
  while (true) {
    std::this_thread::sleep_for(100ms);
  }

  return 0;
}
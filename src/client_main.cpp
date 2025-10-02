#include <chrono>
#include <thread>

#include "client/states/client_state_manager.hpp"
#include "common/chat_utils.hpp"

using namespace grpc;

using namespace std::chrono_literals;

int main(int argc, char* argv[]) {
  logger = std::shared_ptr<quill::Logger>{
      quill::Frontend::create_or_get_logger(getenv("USER") ? getenv("USER") : "unknown_user",
                                            quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_client"))};

  logger->set_log_level(quill::LogLevel::TraceL3);
  quill::Backend::start();

  auto& csm = client_state_manager::get_instance();

  std::cout << "Start of client\n";
  csm.start();

  // Main client loop
  while (true) {
    std::this_thread::sleep_for(100ms);
  }

  return 0;
}
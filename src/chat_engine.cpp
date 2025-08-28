#include <stdio.h>
#include <rpc/client.h>

#include <tinyfsm/tinyfsm.hpp>

auto main(int argc, char* argv[]) -> int {
  auto client = rpc::client("localhost", 10001);
  printf("Hello, Chat Engine!\n");
  return 0;
}
#include "async/async.h"
#include "async/profiling.h"

#include <iostream>
#include <chrono>

namespace ctx = boost::context;
using namespace std::chrono_literals;

void trace(const char* path, const char* msg) {
  std::cout << "t=" << std::this_thread::get_id() << ", p=" << path << ": " << msg << "\n";
  fflush(stdout);
}
void tracecont(const char* path, const char* msg, const ctx::continuation& c) {
  std::cout << "t=" << std::this_thread::get_id() << ", p=" << path << ": " << msg << "=" << c
            << "\n";
  fflush(stdout);
}

void absorb(const char* path, ctx::continuation&& c) {
  ctx::continuation empty;
  tracecont(path, "absorbing", c);
  c.swap(empty);
}

void consume(const char* path, ctx::continuation& c) {
  while (c) {
    tracecont(path, "consume resuming", c);
    c = c.resume();
  }
}

int long_task(int input) {
  profiling::zone zone{CURRENT_LOCATION()};
  int result = input;
  for (int i = 0; i < 3; i++) {
    profiling::sleep_for(110ms);
    result += 1;
  }
  return result;
}

int greeting_task() {
  profiling::zone zone{CURRENT_LOCATION()};
  std::cout << "Hello world! Have an int.\n";
  profiling::sleep_for(130ms);
  return 13;
}

int concurrency_example() {
  profiling::zone zone{CURRENT_LOCATION()};
  async::async_oper_state<int> op;
  op.spawn([]() -> int { return long_task(0); });
  auto x = greeting_task();
  auto y = op.await();
  return x + y;
}

int main() {
  profiling::zone zone{CURRENT_LOCATION()};
  profiling::sleep_for(100ms);
  int r = concurrency_example();
  std::cout << r << "\n";

  profiling::sleep_for(100ms);
  trace(__FUNCTION__, "expecting a crash here, while joining threads");
  // TODO: handle this gracefully
  async::global_thread_pool().clear();

  return 0;
}
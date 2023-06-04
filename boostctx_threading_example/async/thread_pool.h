#pragma once

#include "profiling.h"

#include <thread>
#include <vector>

namespace async {

class thread_pool {
public:
  template <typename Fn> void start_thread(Fn&& f) {
    threads_.emplace_back([f = std::forward<Fn>(f)] {
      profiling::set_cur_thread_name("worker thread");
      f();
    });
  }

  void clear() {
    for (auto& t : threads_) {
      t.join();
    }
    threads_.clear();
  }

private:
  std::vector<std::thread> threads_;
};

} // namespace async
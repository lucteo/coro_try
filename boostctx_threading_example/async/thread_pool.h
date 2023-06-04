#pragma once

#include <thread>
#include <vector>

namespace async {

class thread_pool {
public:
  template <typename Fn> void start_thread(Fn&& f) { threads_.emplace_back(f); }

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
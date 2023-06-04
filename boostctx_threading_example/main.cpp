#include <boost/context/continuation.hpp>
#include <iostream>
#include <thread>
#include <vector>

namespace ctx = boost::context;

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

static thread_pool g_thread_pool;

template <typename T> class async_oper_state {
public:
  async_oper_state() = default;
  ~async_oper_state() = default;
  async_oper_state(const async_oper_state&) = delete;
  async_oper_state(async_oper_state&&) = delete;

  template <typename Fn> void spawn(Fn&& f) {
    auto f_cont = [this,
                   f = std::forward<Fn>(f)](ctx::continuation&& thread_cont) -> ctx::continuation {
      this->thread_cont_ = std::move(thread_cont);
      res_ = f();
      auto c = this->on_async_complete();
      if (c) {
        c = c.resume();
      }
      return std::move(this->thread_cont_);
    };
    g_thread_pool.start_thread(
        [this, f_cont = std::move(f_cont)] { this->cont_ = ctx::callcc(std::move(f_cont)); });
  }

  T await() {
    on_main_complete();
    return res_;
  }

private:
  enum class sync_state {
    both_working,
    first_finishing,
    first_finished,
    second_finished,
  };

  ctx::continuation cont_;
  T res_;
  std::atomic<sync_state> sync_state_{sync_state::both_working};
  ctx::continuation main_cont_;
  ctx::continuation thread_cont_;

  ctx::continuation on_async_complete() {
    sync_state expected{sync_state::both_working};
    if (sync_state_.compare_exchange_strong(expected, sync_state::first_finished)) {
      // We are first to arrive at completion.
      // There is nothing for this thread to do here, we can safely exit.
      return {};
    } else {
      // If the main thread is currently finishing, wait for it to finish.
      while (sync_state_.load() != sync_state::first_finished)
        ; // wait
      // TODO: exponential backoff

      // We are the last to arrive at completion.
      // The main thread set the continuation point; we need to jump there.
      return std::move(main_cont_);
    }
  }

  void on_main_complete() {
    auto c = ctx::callcc([this](ctx::continuation&& await_cc) -> ctx::continuation {
      sync_state expected{sync_state::both_working};
      if (sync_state_.compare_exchange_strong(expected, sync_state::first_finishing)) {
        // We are first to arrive at completion.
        // Store the continuation to move past await.
        this->main_cont_ = std::move(await_cc);
        // We are done "finishing"
        sync_state_ = sync_state::first_finished;
        return std::move(this->thread_cont_);
      } else {
        // The async thread finished; we can continue directly.
        return await_cc;
      }
    });
    // We are here if both threads finish; but we don't know which thread finished last and is
    // currently executing this.
  }
};

int long_task(int input) {
  int result = input;
  for (int i = 0; i < 3; i++) {
    sleep(1);
    result += 1;
  }
  return result;
}

int greeting_task() {
  std::cout << "Hello world! Have an int.\n";
  sleep(1);
  return 13;
}

int concurrency_example() {
  async_oper_state<int> op;
  op.spawn([]() -> int { return long_task(0); });
  auto x = greeting_task();
  auto y = op.await();
  return x + y;
}

int main() {
  int r = concurrency_example();
  std::cout << r << "\n";

  trace(__FUNCTION__, "expecting a crash here, while joining threads");
  // TODO: handle this gracefully
  g_thread_pool.clear();

  return 0;
}
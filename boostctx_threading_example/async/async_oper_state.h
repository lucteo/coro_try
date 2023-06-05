#pragma once

#include "context/continuation.h"
#include "global_thread_pool.h"

namespace async {

template <typename T> class async_oper_state {
public:
  async_oper_state() = default;
  ~async_oper_state() = default;
  async_oper_state(const async_oper_state&) = delete;
  async_oper_state(async_oper_state&&) = delete;

  template <typename Fn> void spawn(Fn&& f) {
    auto f_cont = [this, f = std::forward<Fn>(f)](
                      context::continuation&& thread_cont) -> context::continuation {
      this->thread_cont_ = std::move(thread_cont);
      res_ = f();
      auto c = this->on_async_complete();
      if (c) {
        c = c.resume();
      }
      return std::move(this->thread_cont_);
    };
    global_thread_pool().start_thread(
        [this, f_cont = std::move(f_cont)] { this->cont_ = context::callcc(std::move(f_cont)); });
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

  context::continuation cont_;
  T res_;
  std::atomic<sync_state> sync_state_{sync_state::both_working};
  context::continuation main_cont_;
  context::continuation thread_cont_;

  context::continuation on_async_complete() {
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
    auto c = context::callcc([this](context::continuation&& await_cc) -> context::continuation {
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
    (void) c;
    // We are here if both threads finish; but we don't know which thread finished last and is
    // currently executing this.
  }
};

} // namespace async
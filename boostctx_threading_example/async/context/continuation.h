#pragma once

// TODO: can we include less than this?
#include <boost/context/continuation.hpp>

namespace async {
namespace context {

class continuation {
  boost::context::continuation underlying_continuation_;

public:
  explicit continuation(boost::context::continuation&& c)
      : underlying_continuation_(std::move(c)) {}

  continuation() noexcept = default;
  ~continuation() = default;

  continuation(continuation&&) noexcept = default;
  continuation& operator=(continuation&&) noexcept = default;

  continuation(const continuation&) noexcept = delete;
  continuation& operator=(const continuation&) noexcept = delete;

  continuation resume() {
    {
      profiling::zone zone{CURRENT_LOCATION()};
      (void)zone;
    }
    return continuation{underlying_continuation_.resume()};
  }

  // TODO: resume_with
  // TODO: operator <<
  // TODO: swap

  explicit operator bool() const noexcept { return bool{underlying_continuation_}; }
  bool operator!() const noexcept { return !bool{underlying_continuation_}; }

  const boost::context::continuation& underlying() const { return underlying_continuation_; }
  boost::context::continuation&& sink_to_underlying() {
    return std::move(underlying_continuation_);
  }
};

// TODO: concept for (continuation) -> continuation

template <typename F> continuation callcc(F&& f) {
  {
    profiling::zone zone{CURRENT_LOCATION()};
    (void)zone;
  }
  using bcont = boost::context::continuation;
  auto r = boost::context::callcc([f = std::move(f)](bcont&& c) -> bcont {
    return f(continuation{std::move(c)}).sink_to_underlying();
  });
  return continuation{std::move(r)};
}

} // namespace context
} // namespace async
#pragma once

// TODO: can we include less than this?
#include <boost/context/continuation.hpp>

namespace async {
namespace context {

namespace detail {
//! Handle type representing a context.
using context_handle = boost::context::detail::fcontext_t;

context_handle from_boost_continuation(boost::context::continuation& c) {
  context_handle* src = reinterpret_cast<context_handle*>(&c);
  context_handle r = *src;
  *src = nullptr;
  return r;
}
boost::context::continuation to_boost_continuation(context_handle h) {
  return std::move(*reinterpret_cast<boost::context::continuation*>(&h));
}
} // namespace detail

//! A continuation: the code that follows a certain point.
class continuation {
  detail::context_handle handle_;
  using boost_continuation = boost::context::continuation;

public:
  explicit continuation(detail::context_handle h) : handle_(h) {}

  continuation() noexcept = default;
  ~continuation() = default;

  continuation(continuation&&) noexcept = default;
  continuation& operator=(continuation&&) noexcept = default;

  continuation(const continuation&) noexcept = delete;
  continuation& operator=(const continuation&) noexcept = delete;

  continuation resume() {
    (void) profiling::zone{CURRENT_LOCATION()};
    assert(handle_);
    auto r = boost::context::detail::jump_fcontext(std::exchange(handle_, nullptr), nullptr);
    return continuation{r.fctx};
  }

  // TODO: resume_with
  // TODO: operator <<
  // TODO: swap

  explicit operator bool() const noexcept { return handle_ != nullptr; }
  bool operator!() const noexcept { return handle_ == nullptr; }

  detail::context_handle handle() const { return handle_; }
};

// TODO: concept for (continuation) -> continuation

template <typename F> continuation callcc(F&& f) {
  {
    profiling::zone zone{CURRENT_LOCATION()};
    (void)zone;
  }
  using bcont = boost::context::continuation;
  auto r = boost::context::callcc([f = std::move(f)](bcont&& c) -> bcont {
    auto our_in_cont = continuation{detail::from_boost_continuation(c)};
    auto our_out_cont = f(std::move(our_in_cont));
    return detail::to_boost_continuation(our_out_cont.handle());
  });
  return continuation{detail::from_boost_continuation(r)};
}

} // namespace context
} // namespace async
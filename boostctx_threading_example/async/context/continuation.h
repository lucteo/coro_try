#pragma once

// TODO: can we include less than this?
#include <boost/context/continuation.hpp>

namespace async {
namespace context {

//! Handle type representing a context.
//! This can be thpught as a point from which we can continue exection of the program.
using context_handle = boost::context::detail::fcontext_t;

namespace detail {

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

// TODO: concept for (context_handle) -> context_handle

template <typename F> context_handle callcc(F&& f) {
  (void)profiling::zone{CURRENT_LOCATION()};
  using bcont = boost::context::continuation;
  auto r = boost::context::callcc([f = std::move(f)](bcont&& c) -> bcont {
    auto result = f(detail::from_boost_continuation(c));
    return detail::to_boost_continuation(result);
  });
  return detail::from_boost_continuation(r);
}

inline context_handle resume(context_handle cont) {
  (void)profiling::zone{CURRENT_LOCATION()};
  assert(cont);
  return boost::context::detail::jump_fcontext(cont, nullptr).fctx;
}

} // namespace context
} // namespace async
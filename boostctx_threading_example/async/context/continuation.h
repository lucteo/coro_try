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

//! Concept for the context function types.
//! This matches all invocables with the signature `(context_handle) -> context_handle`.
template <typename F>
concept context_function = requires(F&& f, context_handle continuation) {
  { std::invoke(std::forward<F>(f), continuation) } -> std::same_as<context_handle>;
};

//! Call with current continuation.
//! Takes the context of the code immediatelly following this function call, and passes it to the
//! given context function. The given function is executed in a new stack context. We can suspend
//! the context and resume other context, or the given context.
inline context_handle callcc(context_function auto&& f) {
  (void)profiling::zone{CURRENT_LOCATION()};
  using bcont = boost::context::continuation;
  auto r = boost::context::callcc([f = std::move(f)](bcont&& c) -> bcont {
    auto result = std::invoke(std::move(f), detail::from_boost_continuation(c));
    return detail::to_boost_continuation(result);
  });
  return detail::from_boost_continuation(r);
}

//! Resumes the given continuation.
//! The current execution is interrupted, and the program continues from the given continuation
//! point. Returns the context that has been suspended.
inline context_handle resume(context_handle continuation) {
  (void)profiling::zone{CURRENT_LOCATION()};
  assert(continuation);
  return boost::context::detail::jump_fcontext(continuation, nullptr).fctx;
}

} // namespace context
} // namespace async
#pragma once

#include "thread_pool.h"

namespace async {

inline thread_pool& global_thread_pool() {
  static thread_pool instance;
  return instance;
}

} // namespace async
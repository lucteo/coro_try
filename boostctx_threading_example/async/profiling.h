#pragma once

#if USE_TRACY

#include <tracy_interface.hpp>

#define CURRENT_LOCATION()                                                                         \
  tracy_interface::location { nullptr, __FUNCTION__, __FILE__, __LINE__, 0 }

namespace profiling {

struct zone {
  explicit zone(const tracy_interface::location& loc) { tracy_interface::emit_zone_begin(&loc); }

  ~zone() { tracy_interface::emit_zone_end(); }
};
} // namespace profiling

#else

#define CURRENT_LOCATION() 0

namespace profiling {

struct zone {
  explicit zone(int dummy) {}

  ~zone() = default;
};
} // namespace profiling

#endif

namespace profiling {

void sleep(int seconds) {
  zone zone{CURRENT_LOCATION()};
  ::sleep(seconds);
}

} // namespace profiling

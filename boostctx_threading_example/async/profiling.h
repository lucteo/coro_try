#pragma once

#include <unistd.h>

#if USE_TRACY

#include <tracy_interface.hpp>

#define CURRENT_LOCATION()                                                                         \
  [](const char* f) -> tracy_interface::location {                                                 \
    static tracy_interface::location l{nullptr, f, __FILE__, __LINE__, 0};                         \
    return l;                                                                                      \
  }(__FUNCTION__)
#define CURRENT_LOCATION_N(name)                                                                   \
  [](const char* f) -> tracy_interface::location {                                                 \
    static tracy_interface::location l{name, f, __FILE__, __LINE__, 0};                            \
    return l;                                                                                      \
  }(__FUNCTION__)
#define CURRENT_LOCATION_C(color)                                                                  \
  [](const char* f) -> tracy_interface::location {                                                 \
    static tracy_interface::location l{nullptr, f, __FILE__, __LINE__,                             \
                                       static_cast<uint32_t>(color)};                              \
    return l;                                                                                      \
  }(__FUNCTION__)
#define CURRENT_LOCATION_NC(name, color)                                                           \
  [](const char* f) -> tracy_interface::location {                                                 \
    static tracy_interface::location l{name, f, __FILE__, __LINE__, static_cast<uint32_t>(color)}; \
    return l;                                                                                      \
  }(__FUNCTION__)

namespace profiling {

struct zone {
  explicit zone(const tracy_interface::location& loc) { tracy_interface::emit_zone_begin(&loc); }

  ~zone() { tracy_interface::emit_zone_end(); }
};

inline void set_cur_thread_name(const char* static_name) {
  tracy_interface::set_cur_thread_name(static_name);
}

} // namespace profiling

#else

#define CURRENT_LOCATION() 0
#define CURRENT_LOCATION_N(name) 0
#define CURRENT_LOCATION_C(color) 0
#define CURRENT_LOCATION_NC(name, color) 0

namespace profiling {

struct zone {
  explicit zone(int dummy) {}

  ~zone() = default;
};

inline void set_cur_thread_name(const char* static_name) {}

} // namespace profiling

#endif

namespace profiling {

enum class color {
  automatic = 0,
  gray = 0x808080,
};

inline void sleep(int seconds) {
  zone zone{CURRENT_LOCATION_C(color::gray)};
  ::sleep(seconds);
}

} // namespace profiling

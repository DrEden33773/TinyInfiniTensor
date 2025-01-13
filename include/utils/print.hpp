#pragma once

#include "fmt/core.h"

namespace infini {

template <typename... T>
FMT_INLINE void println(fmt::format_string<T...> fmt, T &&...args) {
  return fmt::println(stdout, fmt, static_cast<T &&>(args)...);
}
template <typename... T>
FMT_INLINE void print(fmt::format_string<T...> fmt, T &&...args) {
  return fmt::print(stdout, fmt, static_cast<T &&>(args)...);
}
template <typename... T>
FMT_INLINE void eprintln(fmt::format_string<T...> fmt, T &&...args) {
  return fmt::println(stderr, fmt, static_cast<T &&>(args)...);
}
template <typename... T>
FMT_INLINE void eprint(fmt::format_string<T...> fmt, T &&...args) {
  return fmt::print(stderr, fmt, static_cast<T &&>(args)...);
}

} // namespace infini
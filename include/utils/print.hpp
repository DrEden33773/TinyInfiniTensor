#pragma once

#include "fmt/core.h"
#include <iterator>
#include <vector>

namespace infini {

template <typename T>
FMT_INLINE std::string to_string(const std::vector<T> &vec) {
  std::string s{"["};
  auto it = std::back_inserter(s);

  for (const auto &v : vec) {
    it = fmt::format_to(it, "{}", v);
    if (&v != &vec.back())
      it = fmt::format_to(it, ", ");
  }
  it = fmt::format_to(it, "]");

  return s;
}

template <typename T> FMT_INLINE std::string to_string(const T &val) {
  return std::to_string(val);
}

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
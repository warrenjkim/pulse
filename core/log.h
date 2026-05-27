#pragma once

#include <_stdio.h>
#include <unistd.h>

#include <cstddef>
#include <cstdio>
#include <functional>
#include <iostream>
#include <source_location>
#include <sstream>
#include <string_view>

namespace pulse {

namespace internal {

// Pick a stable ANSI color for a source file. Same file -> same color,
// always. Uses the 256-color cube; avoids the low/dark and near-white
// ends so every choice is readable on a dark terminal.
inline int file_color(std::string_view file) {
  std::size_t h = std::hash<std::string_view>{}(file);
  // 256-color palette: 16..231 is the 6x6x6 color cube. Bias into the
  // brighter, more saturated region so colors are distinguishable.
  return 16 + static_cast<int>(h % 216);
}

}  // namespace internal

// Streams a single Log line to std::cerr, prefixed with source location. The
// line is assembled in full and written once on destruction.
//
// Usage:
//
//   pulse::Log() << "your log line";
//   pulse::Log() << "parse error: " << err.message;
//
// NOTE: No trailing '\n' needed; one is appended automatically.
class Log {
 public:
  explicit Log(std::source_location loc = std::source_location::current())
      : loc_(loc) {}

  ~Log() {
    std::string_view file = loc_.file_name();
    static const bool tty = isatty(fileno(stderr));

    if (tty) {
      std::clog << "\x1b[38;5;" << internal::file_color(file) << "m" << "["
                << file << ":" << loc_.line() << "]" << "\x1b[0m " << buf_.str()
                << "\n"
                << std::flush;
    } else {
      std::clog << "[" << file << ":" << loc_.line() << "] " << buf_.str()
                << "\n"
                << std::flush;
    }
  }

  // Not movable or copyable.
  Log(const Log&) = delete;
  Log& operator=(const Log&) = delete;

  template <typename T>
  Log& operator<<(const T& value) {
    buf_ << value;
    return *this;
  }

 private:
  std::source_location loc_;
  std::ostringstream buf_;
};

}  // namespace pulse

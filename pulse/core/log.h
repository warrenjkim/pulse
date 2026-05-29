#pragma once

#include <iostream>
#include <ostream>
#include <source_location>
#include <sstream>

namespace pulse {

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
    std::clog << "[" << loc_.file_name() << ":" << loc_.line() << "] "
              << buf_.str() << "\n"
              << std::flush;
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

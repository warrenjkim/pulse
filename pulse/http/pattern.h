#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "pulse/core/container_stringify.h"
#include "pulse/core/enum_macros.h"
#include "pulse/core/result.h"
#include "pulse/core/stringify.h"
#include "pulse/http/parameters.h"
#include "pulse/strings/cat.h"

namespace pulse::http {

#define PATTERN_SEGMENT_TYPE_TABLE(X) \
  X(kPattern, "PATTERN")              \
  X(kCapture, "CAPTURE")

class Pattern {
 public:
  using Captures = Parameters;

  static Result<Pattern> Make(std::string_view pattern);

  std::optional<Captures> Match(std::string_view path) const;

  size_t segments() const { return pattern_.size(); }

  int captures() const { return captures_; }

 private:
  struct Segment {
    PULSE_ENUM(Type, PATTERN_SEGMENT_TYPE_TABLE);
    Type type;
    std::string token;
  };

  friend struct pulse::Stringify<Pattern::Segment::Type>;
  friend struct pulse::Stringify<Pattern::Segment>;
  friend struct pulse::Stringify<Pattern>;

  explicit Pattern(std::vector<Segment> pattern, int captures)
      : pattern_(std::move(pattern)), captures_(captures) {}

  std::vector<Segment> pattern_;
  int captures_;
};

}  // namespace pulse::http

PULSE_ENUM_TO_STRING(pulse::http::Pattern::Segment::Type,
                     PATTERN_SEGMENT_TYPE_TABLE);

template <>
struct pulse::Stringify<pulse::http::Pattern::Segment> {
  static std::string to_string(const pulse::http::Pattern::Segment& segment) {
    return strings::Cat("Segment{.type=", segment.type,
                        ",.token=", segment.token, "}");
  }
};

template <>
struct pulse::Stringify<pulse::http::Pattern> {
  static std::string to_string(const pulse::http::Pattern& pattern) {
    return strings::Cat(
        "Pattern{.pattern_=",
        pulse::Stringify<std::vector<pulse::http::Pattern::Segment>>::ToString(
            pattern.pattern_),
        ",.captures_=", pattern.captures_, "}");
  }
};

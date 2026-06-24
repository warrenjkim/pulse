#include "pulse/http/pattern.h"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/http/parameters.h"
#include "pulse/strings/cat.h"
#include "pulse/strings/split.h"

namespace pulse::http {

namespace {

constexpr std::string_view kDelimiter = "/";
constexpr char kCaptureStart = '{';
constexpr char kCaptureEnd = '}';

}  // namespace

pulse::Result<Pattern> Pattern::Make(std::string_view pattern) {
  if (pattern.empty() || pattern.front() != kDelimiter.front()) {
    return pulse::Error{.code = pulse::Error::Code::kInvalidArgument,
                        .message = strings::cat("pattern must start with '",
                                                kDelimiter, "': ", pattern)};
  }

  const std::vector<std::string_view> parts =
      strings::split(pattern, kDelimiter);

  std::unordered_set<std::string_view> captures;
  std::vector<Segment> segments;
  segments.reserve(parts.size());

  for (std::string_view part : parts) {
    if (part.size() > 2 && part.front() == kCaptureStart &&
        part.back() == kCaptureEnd) {
      std::string_view name = part.substr(1, part.size() - 2);
      if (name.contains(kCaptureStart) || name.contains(kCaptureEnd)) {
        return pulse::Error{
            .code = pulse::Error::Code::kInvalidArgument,
            .message = strings::cat("nested or malformed braces in capture: ",
                                    pattern)};
      }

      if (!captures.insert(name).second) {
        return pulse::Error{
            .code = pulse::Error::Code::kInvalidArgument,
            .message = strings::cat("pattern has duplicate capture variable '",
                                    name, "': ", pattern)};
      }

      segments.push_back(
          {.type = Segment::Type::kCapture, .token = std::string(name)});
    } else {
      if (part.contains(kCaptureStart) || part.contains(kCaptureEnd)) {
        return pulse::Error{.code = pulse::Error::Code::kInvalidArgument,
                            .message = strings::cat(
                                "stray brace in literal segment: ", pattern)};
      }

      segments.push_back(
          {.type = Segment::Type::kPattern, .token = std::string(part)});
    }
  }

  return Pattern(std::move(segments), static_cast<int>(captures.size()));
}

std::optional<Pattern::Captures> Pattern::Match(std::string_view path) const {
  Parameters captures;
  const std::vector<std::string_view> parts = strings::split(path, kDelimiter);
  if (parts.size() != pattern_.size()) {
    return std::nullopt;
  }

  for (size_t i = 0; i < pattern_.size(); i++) {
    const auto& [type, token] = pattern_[i];
    if (type == Segment::Type::kPattern && token != parts[i]) {
      return std::nullopt;
    } else if (type == Segment::Type::kCapture) {
      captures[token] = std::string(parts[i]);
    }
  }

  return captures;
}

}  // namespace pulse::http

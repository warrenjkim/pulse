#include "http/pattern.h"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "dsa/error.h"
#include "dsa/result.h"
#include "strings/split.h"

namespace pulse::http {

namespace {

constexpr std::string_view kDelimiter = "/";
constexpr char kCaptureStart = '{';
constexpr char kCaptureEnd = '}';

}  // namespace

pulse::Result<Pattern> Pattern::Make(std::string_view pattern) {
  if (pattern.empty() || pattern.front() != kDelimiter.front()) {
    return pulse::Error{.code = pulse::Error::Code::kInvalidArgument,
                        .message = "pattern must start with '" +
                                   std::string(kDelimiter) +
                                   "': " + std::string(pattern)};
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
            .message = "nested or malformed braces in capture: " +
                       std::string(pattern)};
      }

      if (!captures.insert(name).second) {
        return pulse::Error{
            .code = pulse::Error::Code::kInvalidArgument,
            .message = "pattern has duplicate capture variable '" +
                       std::string(name) + "': " + std::string(pattern)};
      }

      segments.push_back(
          {.type = Segment::Type::kCapture, .token = std::string(name)});
    } else {
      if (part.contains(kCaptureStart) || part.contains(kCaptureEnd)) {
        return pulse::Error{.code = pulse::Error::Code::kInvalidArgument,
                            .message = "stray brace in literal segment: " +
                                       std::string(pattern)};
      }

      segments.push_back(
          {.type = Segment::Type::kPattern, .token = std::string(part)});
    }
  }

  return Pattern(std::move(segments), static_cast<int>(captures.size()));
}

std::optional<Pattern::Captures> Pattern::match(std::string_view path) const {
  std::unordered_map<std::string, std::string> captures;
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

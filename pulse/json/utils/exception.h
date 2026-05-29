#pragma once

#include <exception>
#include <string>

namespace pulse {
namespace json {

class JsonException : public std::exception {
 public:
  explicit JsonException(const std::string& message) : message_(message) {}

  const char* what() const noexcept override { return message_.c_str(); }

 private:
  std::string message_;
};

class ParseException final : public JsonException {
  using JsonException::JsonException;
};

class BadAccessException final : public JsonException {
  using JsonException::JsonException;
};

// TODO(implement iterators)
class NonIterableTypeException final : public JsonException {
  using JsonException::JsonException;
};

}  // namespace json
}  // namespace pulse

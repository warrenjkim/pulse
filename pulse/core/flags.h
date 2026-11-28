#pragma once

#include <charconv>
#include <concepts>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <system_error>

#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/strings/cat.h"

namespace pulse {

// Specialize to make a type parseable as a CLI flag:
//
//   template <>
//   struct pulse::FlagParser<YourType> {
//     static pulse::Result<YourType> Parse(std::string_view raw);
//   };
template <typename T>
struct FlagParser;

template <typename T>
concept FlagParseable = requires(std::string_view raw) {
  { FlagParser<T>::Parse(raw) } -> std::same_as<Result<T>>;
};

// Lightweight non-owning view over `main`'s `(argc, argv)` for extracting CLI
// flags.
//
// Accepts both `--flag=value` and `--flag value` forms. Stops scanning at `--`.
// If a flag appears multiple times, the last occurrence wins.
//
// NOTE: `argv` must outlive this `Flags` instance and any returned
// `std::string_view`.
class Flags {
 public:
  Flags(int argc, const char* const* argv);

  // Returns the raw value of `name`, or `std::nullopt` if absent.
  std::optional<std::string_view> Find(std::string_view name) const;

  // Returns the raw value of `name`, or `default_value` if absent.
  std::string_view Get(std::string_view name,
                       std::string_view default_value = "") const;

  // Parses the value of `name` via `FlagParser<T>`, returning `default_value`
  // if absent or an `Error` if the value fails to parse.
  template <FlagParseable T>
  Result<T> Get(std::string_view name, T default_value) const;

  // Returns true if `name` appears before `--`, either as a bare flag
  // (`--flag`) or with a value (`--flag=value`).
  bool Has(std::string_view name) const;

 private:
  std::span<const char* const> args_;
};

// Implementation details below;

namespace internal {

template <typename T>
concept FlagCharLike = std::same_as<T, char> || std::same_as<T, wchar_t> ||
                       std::same_as<T, char8_t> || std::same_as<T, char16_t> ||
                       std::same_as<T, char32_t>;

template <typename T>
concept FlagNumeric = (std::integral<T> || std::floating_point<T>) &&
                      !std::same_as<T, bool> && !FlagCharLike<T>;

}  // namespace internal

template <internal::FlagNumeric T>
struct FlagParser<T> {
  static Result<T> Parse(std::string_view raw) {
    T value{};
    const char* end = raw.data() + raw.size();
    if (auto [ptr, ec] = std::from_chars(raw.data(), end, value);
        ec != std::errc{} || ptr != end) {
      return Error{
          .code = Error::Code::kInvalidArgument,
          .message = strings::Cat("invalid numeric value: ", raw),
      };
    }

    return value;
  }
};

template <>
struct FlagParser<bool> {
  static Result<bool> Parse(std::string_view raw) {
    if (raw == "true" || raw == "1") {
      return true;
    }

    if (raw == "false" || raw == "0") {
      return false;
    }

    return Error{
        .code = Error::Code::kInvalidArgument,
        .message = strings::Cat("invalid bool value: ", raw),
    };
  }
};

template <>
struct FlagParser<std::string_view> {
  static Result<std::string_view> Parse(std::string_view raw) { return raw; }
};

template <>
struct FlagParser<std::string> {
  static Result<std::string> Parse(std::string_view raw) {
    return std::string(raw);
  }
};

template <FlagParseable T>
Result<T> Flags::Get(std::string_view name, T default_value) const {
  std::optional<std::string_view> raw = Find(name);
  if (!raw.has_value()) {
    return default_value;
  }

  Result<T> parsed = FlagParser<T>::Parse(*raw);
  if (!parsed.ok()) {
    return Error{
        .code = parsed.error().code,
        .message = strings::Cat("flag ", name, ": ", parsed.error().message),
    };
  }

  return parsed;
}

}  // namespace pulse

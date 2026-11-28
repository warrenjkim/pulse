#include "pulse/core/flags.h"

#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

namespace pulse {

Flags::Flags(int argc, const char* const* argv)
    : args_(argv != nullptr && argc > 1
                ? std::span<const char* const>(argv + 1,
                                               static_cast<size_t>(argc - 1))
                : std::span<const char* const>{}) {}

std::optional<std::string_view> Flags::Find(std::string_view name) const {
  if (name.empty()) {
    return std::nullopt;
  }

  std::optional<std::string_view> result;
  for (size_t i = 0; i < args_.size(); i++) {
    std::string_view arg(args_[i]);
    if (arg == "--") {
      break;
    }

    if (arg == name) {
      if (i + 1 < args_.size()) {
        result = std::string_view(args_[++i]);
      }

      continue;
    }

    if (arg.starts_with(name) && arg.size() > name.size() &&
        arg[name.size()] == '=') {
      result = arg.substr(name.size() + 1);
    }
  }

  return result;
}

std::string_view Flags::Get(std::string_view name,
                            std::string_view default_value) const {
  return Find(name).value_or(default_value);
}

bool Flags::Has(std::string_view name) const {
  if (name.empty()) {
    return false;
  }

  for (const char* raw : args_) {
    std::string_view arg(raw);
    if (arg == "--") {
      break;
    }

    if (arg == name || (arg.starts_with(name) && arg.size() > name.size() &&
                        arg[name.size()] == '=')) {
      return true;
    }
  }

  return false;
}

}  // namespace pulse

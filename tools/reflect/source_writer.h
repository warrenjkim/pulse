#pragma once

#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace pulse::reflect::plugin {

inline constexpr size_t kIndentWidth = 2;

class SourceWriter {
 public:
  template <std::convertible_to<std::string_view>... Pieces>
  void Line(const Pieces&... pieces) {
    if constexpr (sizeof...(Pieces) > 0) {
      out_.append(indent_ * kIndentWidth, ' ');
      (out_.append(std::string_view(pieces)), ...);
    }

    out_ += '\n';
  }

  void Indent() { ++indent_; }

  void Dedent() { --indent_; }

  std::string operator*() && { return std::move(out_); }

 private:
  std::string out_;
  size_t indent_ = 0;
};

}  // namespace pulse::reflect::plugin

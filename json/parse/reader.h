#pragma once

#include <cstddef>
#include <optional>
#include <string>

namespace pulse {
namespace json {

class Reader {
 public:
  explicit Reader(std::string json) : json_(std::move(json)), pos_(0) {}

  bool eof() const { return pos_ >= json_.length(); }

  size_t tell() const { return pos_; }

  char peek() const { return json_[pos_]; }

  char get() { return json_[pos_++]; }

  bool expect(char c) { return json_[pos_] == c && ++pos_; }

  std::string substr(size_t start,
                     std::optional<size_t> length = std::nullopt) const {
    if (length) {
      return json_.substr(start, *length);
    }

    return json_.substr(start);
  }

 private:
  std::string json_;
  size_t pos_;
};

}  // namespace json
}  // namespace pulse

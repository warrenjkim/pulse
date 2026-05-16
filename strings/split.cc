#include "strings/split.h"

#include <string>
#include <string_view>
#include <vector>

namespace pulse::strings {

std::vector<std::string_view> split(std::string_view string,
                                    std::string_view delimiter) {
  if (delimiter.empty()) {
    return {string};
  }

  size_t cursor = 0;
  size_t match = string.find(delimiter, cursor);
  std::vector<std::string_view> parts;
  while (match != std::string::npos) {
    parts.push_back(string.substr(cursor, match - cursor));
    cursor = match + delimiter.size();
    match = string.find(delimiter, cursor);
  }

  if (cursor <= string.size()) {
    parts.push_back(string.substr(cursor));
  }

  return parts;
}

}  // namespace pulse::strings

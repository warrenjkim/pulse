#pragma once

#include <string_view>
#include <vector>

namespace pulse::strings {

// Splits `string` on each occurrence of `delimiter`, returning a vector of
// views into `string`. Empty tokens are preserved: i.e. a leading, trailing, or
// consecutive delimiter produces an empty `std::string_view` at that position.
//
// NOTE: `string` must outlive the returned views.
std::vector<std::string_view> split(std::string_view string,
                                    std::string_view delimiter);

}  // namespace pulse::strings

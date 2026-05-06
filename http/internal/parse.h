#pragma once

#include <string_view>
#include <vector>

#include "dsa/result.h"
#include "http/request.h"

namespace pulse::http {

// TODO(might want to move this out into dsa)
// currently public just to make it easier to test
std::vector<std::string_view> split(std::string_view string,
                                    std::string_view delimiter);

Result<Request> parse(std::string_view raw);

}  // namespace pulse::http

#pragma once

#include <string>
#include <string_view>

#include "pulse/core/result.h"

namespace pulse::http {

// Decodes percent-encoded characters (%XX) in a URL path segment. Returns
// `pulse::Error::kInvalidArgument` if the input contains a malformed sequence,
// the decoded sequence otherwise.
Result<std::string> DecodePercent(std::string_view input);

}  // namespace pulse::http

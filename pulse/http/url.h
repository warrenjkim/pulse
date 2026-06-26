#pragma once

#include <string>
#include <string_view>

#include "pulse/core/result.h"

namespace pulse::http {

// Decodes percent-encoded characters (%XX) in a URL path segment.
// Does NOT decode '+' as space — that is form-specific behavior.
// Returns kInvalidArgument if the input contains a malformed percent sequence.
Result<std::string> DecodePercent(std::string_view input);

}  // namespace pulse::http

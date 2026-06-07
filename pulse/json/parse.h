#pragma once

#include <string_view>

#include "pulse/core/result.h"
#include "pulse/json/value.h"

namespace pulse::json {

Result<value> parse(std::string_view json);

}  // namespace pulse::json

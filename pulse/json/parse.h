#pragma once

#include <string_view>

#include "pulse/core/result.h"
#include "pulse/json/value.h"

namespace pulse::json {

Result<Value> Parse(std::string_view json);

}  // namespace pulse::json

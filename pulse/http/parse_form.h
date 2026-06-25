#pragma once

#include <string_view>

#include "pulse/core/result.h"
#include "pulse/http/parameters.h"

namespace pulse::http {

Result<Parameters> ParseForm(std::string_view body);

}

#pragma once

#include <string>

#include "http/response.h"

namespace pulse::http {

std::string serialize(const Response& response);

}  // namespace pulse::http

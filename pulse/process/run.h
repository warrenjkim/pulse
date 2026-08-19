#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "pulse/core/result.h"
#include "pulse/process/output.h"

namespace pulse::process {

// Run `cmd` as a child process with the given `args`, blocking until it
// exits. Captures the output into the `Output` struct. Returns an `Error` if
// the process couldn't be spawned at all. Otherwise, returns an `Output`
// struct.
//
// NOTE: Both streams are fully buffered in memory as `std::string`s. Avoid
// using this function for very large or unbounded output.
Result<Output> Run(std::string_view cmd, std::vector<std::string> args = {});

}  // namespace pulse::process

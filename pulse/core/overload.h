#pragma once

namespace pulse {

// A variadic overload for std::visit.
//
// Example Usage:
//
//   std::visit(Overload{
//       [](int i) { ... },
//       [](std::string s) { ... },
//       ...
//   }, variant);
template <typename... Ts>
struct Overload : Ts... {
  using Ts::operator()...;
};

}  // namespace pulse

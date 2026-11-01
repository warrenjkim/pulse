#pragma once

#include <string>
#include <vector>

#include "tools/reflect/record.h"

namespace pulse::reflect::plugin {

// Renders a header holding the reflection source of each of `records`, each
// within its class's namespaces.
//
// ```
//   namespace ns {
//
//   struct pulse_reflect A {
//     int i;
//   };
//
//   }
// ```
//
// renders as:
//
// ```
//   #pragma once
//
//   #include "pulse/reflect/mirror.h"
//
//   namespace ns {
//   [[maybe_unused]] consteval auto PulseReflect(const ::ns::A*) {
//     struct PulseField0 {
//       using Type = decltype(::ns::A::i);
//       static consteval const char* Name() { return "i"; }
//       static constexpr const Type& Value(const ::ns::A& v) { return v.i; }
//     };
//     return ::pulse::reflect::Mirror<::ns::A, PulseField0>{};
//   }
//   }  // namespace ns
// ```
std::string RenderHeader(const std::vector<Record>& records);

}  // namespace pulse::reflect::plugin

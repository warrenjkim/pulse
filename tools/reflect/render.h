#pragma once

#include <string>

#include "tools/reflect/record.h"

namespace pulse::reflect::plugin {

// Renders the reflection source injected after `record`'s definition.
//
// ```
//   namespace ns {
//
//   struct A {
//     int i;
//   };
//
//   }
// ```
//
// renders as:
//
// ```
//   consteval auto PulseReflect(const ::ns::A*) {
//     struct PulseField0 {
//       using Type = decltype(::ns::A::i);
//       static consteval ::std::string_view Name() { return "i"; }
//       static constexpr const Type& Value(const ::ns::A& v) { return v.i; }
//     };
//     return ::pulse::reflect::Mirror<::ns::A, PulseField0>{};
//   }
// ```
std::string Render(const Record& record);

}  // namespace pulse::reflect::plugin

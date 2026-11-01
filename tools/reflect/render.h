#pragma once

#include <string>
#include <vector>

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
std::string RenderRecord(const Record& record);

// Renders a header holding the reflection source of each of `records`, each
// within its class's namespaces.
std::string RenderHeader(const std::vector<Record>& records);

}  // namespace pulse::reflect::plugin

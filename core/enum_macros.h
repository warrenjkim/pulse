#pragma once

#include <string>       // IWYU pragma: export
#include <string_view>  // IWYU pragma: export

#include "core/stringify.h"  // IWYU pragma: export

// Declares an enum class with a kUnknown default value.
//
// Define the enum table:
//
//   #define YOUR_ENUM_TABLE(X)              \
//     X(kSomeValue, "StringRepresentation") \
//     ...
//
// Usage:
//
//   PULSE_ENUM(YourEnumName, YOUR_ENUM_TABLE)
//
// Generates:
//
//   class enum YourEnumName {
//     kUnknown,
//     kSomeValue,
//     ...
//   };
#define PULSE_ENUM(EnumName, TABLE) \
  enum class EnumName { kUnknown, TABLE(PULSE_INTERNAL_DEFINE_ENUM_VALUE) };
#define PULSE_INTERNAL_DEFINE_ENUM_VALUE(enumerator, string) enumerator,

// Specialization of pulse::Stringify for an enum declared with PULSE_ENUM.
//
// * Usage:
//
//   PULSE_ENUM_TO_STRING(YourEnumName, YOUR_ENUM_TABLE)
//
// * Generates:
//
//   template <>
//   struct pulse::Stringify<YourEnumName> {
//     static std::string to_string(const YourEnumName& value);
//   };
//
// * Returns "UNKNOWN" for kUnknown or any unrecognized value.
// * Must be called in the global namespace.
#define PULSE_ENUM_TO_STRING(EnumName, TABLE)             \
  template <>                                             \
  struct pulse::Stringify<EnumName> {                     \
    static std::string to_string(const EnumName& value) { \
      using _E = EnumName;                                \
      switch (value) {                                    \
        case _E::kUnknown:                                \
          return "UNKNOWN";                               \
          TABLE(PULSE_INTERNAL_ENUM_TO_STRING_CASE)       \
      }                                                   \
    }                                                     \
  };
#define PULSE_INTERNAL_ENUM_TO_STRING_CASE(enumerator, string) \
  case _E::enumerator:                                         \
    return string;

// Generates a to_enum() for an enum declared with PULSE_ENUM.
//
// Usage:
//
//   PULSE_STRING_TO_ENUM(YourEnumName, to_your_enum, YOUR_ENUM_TABLE)
//
// Generates:
//
//   inline YourEnumName to_your_enum(std::string_view value);
//
// Returns kUnknown if the string does not match any enumerator.
#define PULSE_STRING_TO_ENUM(EnumName, FnName, TABLE) \
  inline EnumName FnName(std::string_view value) {    \
    using _E = EnumName;                              \
    TABLE(PULSE_INTERNAL_STRING_TO_ENUM_CHECK)        \
    return _E::kUnknown;                              \
  }
#define PULSE_INTERNAL_STRING_TO_ENUM_CHECK(enumerator, string) \
  if (value == string) {                                        \
    return _E::enumerator;                                      \
  }

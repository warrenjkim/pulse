#pragma once

#include <cstdlib>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>

#include "pulse/strings/cat.h"

#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#define PULSE_HAS_CXA_DEMANGLE 1
#endif

namespace pulse {

// Returns the human-readable form of a mangled name, or `mangled`
// unchanged if it cannot be demangled.
inline std::string Demangle(const char* mangled) {
  if (mangled == nullptr) {
    return "";
  }

#ifdef PULSE_HAS_CXA_DEMANGLE
  struct Free {
    void operator()(char* p) const { std::free(p); }
  };

  int status = 0;
  std::unique_ptr<char, Free> demangled(
      abi::__cxa_demangle(mangled, /*output_buffer=*/nullptr,
                          /*length=*/nullptr, &status));

  return status == 0 ? std::string(demangled.get()) : std::string(mangled);
#else
  return std::string(mangled);
#endif
}

// Returns the demangled name of `T`. Demangled once per instantiation.
template <typename T>
const std::string& TypeName() {
  static const std::string name = [] -> std::string {
    if constexpr (std::is_lvalue_reference_v<T>) {
      return strings::Cat(TypeName<std::remove_reference_t<T>>(), "&");
    } else if constexpr (std::is_rvalue_reference_v<T>) {
      return strings::Cat(TypeName<std::remove_reference_t<T>>(), "&&");
    } else if constexpr (std::is_const_v<T> && std::is_volatile_v<T>) {
      return strings::Cat(TypeName<std::remove_cv_t<T>>(), " const volatile");
    } else if constexpr (std::is_const_v<T>) {
      return strings::Cat(TypeName<std::remove_const_t<T>>(), " const");
    } else if constexpr (std::is_volatile_v<T>) {
      return strings::Cat(TypeName<std::remove_volatile_t<T>>(), " volatile");
    } else {
      return Demangle(typeid(T).name());
    }
  }();

  return name;
}

}  // namespace pulse

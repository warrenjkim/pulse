#pragma once

#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>

#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#define PULSE_HAS_CXA_DEMANGLE 1
#endif

namespace pulse {

namespace internal {

// Overrides the demangled spelling of T.
//
// For example:
//
//   std::__1::basic_string<char,...> -> std::string
//
// Container templates don't need aliases because TypeLabel reconstructs
// their names from their element types.
template <typename T>
struct TypeAlias {
  static constexpr std::string_view kName{};
};

#define STRINGIFY(x) #x
#define TYPE_ALIAS(Type)                                       \
  template <>                                                  \
  struct TypeAlias<Type> {                                     \
    static constexpr std::string_view kName = STRINGIFY(Type); \
  }

TYPE_ALIAS(std::string);
TYPE_ALIAS(std::string_view);

#undef TYPE_ALIAS
#undef STRINGIFY

// Removes compiler/library implementation details from a demangled type name.
//
// For example:
//
//   std::__1::vector<int>
//   std::__cxx11::vector<int>
//
// become:
//
//   std::vector<int>
inline std::string StripInlineNamespaces(std::string name) {
  for (std::string_view inline_ns : {"__1::", "__cxx11::"}) {
    for (std::size_t pos = name.find(inline_ns); pos != std::string::npos;
         pos = name.find(inline_ns, pos)) {
      name.erase(pos, inline_ns.size());
    }
  }

  return name;
}

}  // namespace internal

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
      abi::__cxa_demangle(mangled, nullptr, nullptr, &status));
  if (status == 0) {
    return internal::StripInlineNamespaces(demangled.get());
  }

  return std::string(mangled);
#else
  return std::string(mangled);
#endif
}

// Returns the demangled name of `T`.
template <typename T>
const std::string& TypeName() {
  static const std::string name = [] {
    if constexpr (!internal::TypeAlias<T>::kName.empty()) {
      return std::string(internal::TypeAlias<T>::kName);
    } else if constexpr (std::is_lvalue_reference_v<T>) {
      return TypeName<std::remove_reference_t<T>>() + "&";
    } else if constexpr (std::is_rvalue_reference_v<T>) {
      return TypeName<std::remove_reference_t<T>>() + "&&";
    } else if constexpr (std::is_const_v<T> && std::is_volatile_v<T>) {
      return TypeName<std::remove_cv_t<T>>() + " const volatile";
    } else if constexpr (std::is_const_v<T>) {
      return TypeName<std::remove_const_t<T>>() + " const";
    } else if constexpr (std::is_volatile_v<T>) {
      return TypeName<std::remove_volatile_t<T>>() + " volatile";
    } else {
      return Demangle(typeid(T).name());
    }
  }();

  return name;
}

}  // namespace pulse

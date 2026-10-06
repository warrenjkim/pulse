#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

namespace pulse {

namespace internal {

// Overrides the spelling of `T`.
//
// For example:
//
//   std::basic_string<char, std::char_traits<char>, std::allocator<char>>
//     -> std::string
//
// Container templates don't need aliases because `TypeLabel` reconstructs their
// names from their element types.
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

inline constexpr std::string_view kAnonymousNamespace =
    "(anonymous namespace)::";

// True when `name` at `i` begins a qualifier that should be separated from a
// preceding `*` or `&`.
consteval bool IsQualifier(std::string_view name, size_t i) {
  return name.substr(i).starts_with("const") ||
         name.substr(i).starts_with("volatile");
}

// True when the space at `i` precedes a declarator character and should be
// elided.
consteval bool IsSpaceBeforeDeclarator(std::string_view name, size_t i) {
  return name[i] == ' ' && i + 1 < name.size() &&
         (name[i + 1] == '&' || name[i + 1] == '*');
}

// The compiler's spelling of `T`, extracted from `__PRETTY_FUNCTION__`.
//
// NOTE: Preserves cv-qualifiers, references, and non-type template arguments as
// written. An enum keeps its name instead of the underlying value.
template <typename T>
consteval std::string_view RawTypeName() {
  constexpr std::string_view kSignature = __PRETTY_FUNCTION__;

#if defined(__clang__)
  constexpr std::string_view kPrefix = "[T = ";
  constexpr std::string_view kSuffix = "]";
#elif defined(__GNUC__)
  constexpr std::string_view kPrefix = "[with T = ";
  constexpr std::string_view kSuffix = ";";
#endif

  constexpr size_t begin = kSignature.find(kPrefix) + kPrefix.size();
  constexpr size_t end = kSignature.find(kSuffix, begin);

  return kSignature.substr(begin, end - begin);
}

// The compiler's spelling of the non-type template argument `V`.
template <auto V>
consteval std::string_view RawValueName() {
  constexpr std::string_view kSignature = __PRETTY_FUNCTION__;

#if defined(__clang__)
  constexpr std::string_view kPrefix = "[V = ";
  constexpr std::string_view kSuffix = "]";
#elif defined(__GNUC__)
  constexpr std::string_view kPrefix = "[with auto V = ";
  constexpr std::string_view kSuffix = ";";
#endif

  constexpr size_t begin = kSignature.find(kPrefix) + kPrefix.size();
  constexpr size_t end = kSignature.find(kSuffix, begin);

  return kSignature.substr(begin, end - begin);
}

// The length of `name` once anonymous namespace markers are removed and spaces
// are elided.
consteval size_t CleanedSize(std::string_view name) {
  size_t size = 0;
  size_t i = 0;
  char last = '\0';
  while (i < name.size()) {
    if (name.substr(i).starts_with(kAnonymousNamespace)) {
      i += kAnonymousNamespace.size();
      continue;
    }

    if (name[i] == ' ' && i > 0 && name[i - 1] == ',') {
      i++;
      continue;
    }

    if (IsSpaceBeforeDeclarator(name, i)) {
      i++;
      continue;
    }

    if ((last == '*' || last == '&') && IsQualifier(name, i)) {
      size++;
    }

    size++;
    last = name[i];
    i++;
  }

  return size;
}

// Writes `name` into `out` with anonymous namespace markers removed and spaces
// elided.
consteval void Clean(std::string_view name, char* out) {
  size_t j = 0;
  size_t i = 0;
  char last = '\0';
  while (i < name.size()) {
    if (name.substr(i).starts_with(kAnonymousNamespace)) {
      i += kAnonymousNamespace.size();
      continue;
    }

    if (name[i] == ' ' && i > 0 && name[i - 1] == ',') {
      i++;
      continue;
    }

    if (IsSpaceBeforeDeclarator(name, i)) {
      i++;
      continue;
    }

    if ((last == '*' || last == '&') && IsQualifier(name, i)) {
      out[j++] = ' ';
    }

    last = name[i];
    out[j++] = name[i++];
  }
}

// Stores the clean spelling of `T`.
template <typename T>
struct CleanedTypeName {
  static constexpr size_t kSize = CleanedSize(RawTypeName<T>());

  static constexpr std::array<char, kSize + 1> kChars = []() consteval {
    std::array<char, kSize + 1> chars{};
    Clean(RawTypeName<T>(), chars.data());

    return chars;
  }();

  static constexpr std::string_view kName{kChars.data(), kSize};
};

}  // namespace internal

// The name of `T`, with anonymous namespace markers removed and no space after
// a comma in a template argument list.
template <typename T>
constexpr std::string_view TypeName() {
  if constexpr (!internal::TypeAlias<T>::kName.empty()) {
    return internal::TypeAlias<T>::kName;
  } else {
    return internal::CleanedTypeName<T>::kName;
  }
}

// The name of the non-type template argument `V`.
template <auto V>
constexpr std::string_view ValueName() {
  return internal::RawValueName<V>();
}

}  // namespace pulse

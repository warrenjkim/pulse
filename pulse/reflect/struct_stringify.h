#pragma once

#include <cstddef>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "pulse/core/container_stringify.h"
#include "pulse/core/pretty_function.h"
#include "pulse/core/stringify.h"
#include "pulse/reflect/data_sink.h"
#include "pulse/reflect/dump_struct.h"
#include "pulse/reflect/stringify_sink.h"

#if !__has_builtin(__builtin_structured_binding_size)
#error "pulse/reflect/struct_stringify.h requires Clang 21+"
#endif

namespace pulse::reflect::internal {

// A class type that no other `Stringify` specialization claims.
//
// NOTE: Standard library types are excluded so their implementation details
// are never rendered.
template <typename T>
concept AutoStringifiable =
    std::is_class_v<T> && !pulse::TypeName<T>().starts_with("std::") &&
    !std::ranges::range<T> && !pulse::internal::StringLike<T> &&
    !pulse::internal::HasTupleSize<T>;

// A class whose members a structured binding can name: all public, all
// declared in the same class, and no anonymous unions.
template <typename T>
concept Decomposable = requires { __builtin_structured_binding_size(T); };

// Records the names of the outermost struct's fields, in declaration order.
//
// NOTE: The names point into string literals emitted by Clang, so they have
// static storage duration.
class FieldNameSink {
 public:
  void BeginStruct() { depth_++; }
  void EndStruct() { depth_--; }
  void BeginField(std::string_view, std::string_view name) { Add(name); }

  template <typename T>
  void Field(std::string_view name, const T&) {
    Add(name);
  }

  std::vector<std::string_view> names() && { return std::move(names_); }

 private:
  void Add(std::string_view name) {
    if (depth_ == 1) {
      names_.push_back(name);
    }
  }

  int depth_{0};
  std::vector<std::string_view> names_;
};
static_assert(DataSink<FieldNameSink>);

// Returns the names of `T`'s fields, computed once per type.
template <typename T>
const std::vector<std::string_view>& FieldNames(const T& value) {
  static const std::vector<std::string_view> names = [&] {
    FieldNameSink sink;
    DumpStruct<FieldNameSink> dump(&sink);
    __builtin_dump_struct(&value, dump);

    return std::move(sink).names();
  }();

  return names;
}

template <typename F, typename... M>
void Each(F& f, const M&... members) {
  (f(members), ...);
}

// clang-format off
#define PULSE_DECOMPOSE(N, ...)        \
  else if constexpr (kSize == N) {     \
    const auto& [__VA_ARGS__] = value; \
    Each(f, __VA_ARGS__);              \
  }

// Invokes `f` on each member of `value`, in declaration order.
template <Decomposable T, typename F>
void ForEachMember(const T& value, F f) {
  constexpr size_t kSize = __builtin_structured_binding_size(T);

  if constexpr (kSize == 0) {
  }
  PULSE_DECOMPOSE(1, m0)
  PULSE_DECOMPOSE(2, m0, m1)
  PULSE_DECOMPOSE(3, m0, m1, m2)
  PULSE_DECOMPOSE(4, m0, m1, m2, m3)
  PULSE_DECOMPOSE(5, m0, m1, m2, m3, m4)
  PULSE_DECOMPOSE(6, m0, m1, m2, m3, m4, m5)
  PULSE_DECOMPOSE(7, m0, m1, m2, m3, m4, m5, m6)
  PULSE_DECOMPOSE(8, m0, m1, m2, m3, m4, m5, m6, m7)
  PULSE_DECOMPOSE(9, m0, m1, m2, m3, m4, m5, m6, m7, m8)
  PULSE_DECOMPOSE(10, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9)
  PULSE_DECOMPOSE(11, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10)
  PULSE_DECOMPOSE(12, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11)
  PULSE_DECOMPOSE(13, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12)
  PULSE_DECOMPOSE(14, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13)
  PULSE_DECOMPOSE(15, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14)
  PULSE_DECOMPOSE(16, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15)
  PULSE_DECOMPOSE(17, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16)
  PULSE_DECOMPOSE(18, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17)
  PULSE_DECOMPOSE(19, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18)
  PULSE_DECOMPOSE(20, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19)
  PULSE_DECOMPOSE(21, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20)
  PULSE_DECOMPOSE(22, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21)
  PULSE_DECOMPOSE(23, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22)
  PULSE_DECOMPOSE(24, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23)
  PULSE_DECOMPOSE(25, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24)
  PULSE_DECOMPOSE(26, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25)
  PULSE_DECOMPOSE(27, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26)
  PULSE_DECOMPOSE(28, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27)
  PULSE_DECOMPOSE(29, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28)
  PULSE_DECOMPOSE(30, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29)
  PULSE_DECOMPOSE(31, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30)
  PULSE_DECOMPOSE(32, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31)
  else {
    static_assert(false, "structs with more than 32 members are unsupported");
  }
}

#undef PULSE_DECOMPOSE
// clang-format on

// Renders a single member, falling back to its address for pointers that
// have no `Stringify` specialization.
template <typename M>
std::string MemberToString(const M& member) {
  if constexpr (!std::is_pointer_v<M> || pulse::Stringifiable<M>) {
    return pulse::ToString(member);
  } else if constexpr (std::is_function_v<std::remove_pointer_t<M>>) {
    return pulse::ToString(reinterpret_cast<const void*>(member));
  } else {
    return pulse::ToString(static_cast<const void*>(member));
  }
}

// Renders `value` as `{.field=value,...}`, dispatching each member through
// `Stringify`.
template <Decomposable T>
std::string MembersToString(const T& value) {
  const std::vector<std::string_view>& names = FieldNames(value);

  std::string out = "{";
  size_t i = 0;
  ForEachMember(value, [&](const auto& member) {
    out += i == 0 ? "." : ",.";
    out += names[i++];
    out += "=";
    out += MemberToString(member);
  });

  return out + "}";
}

// Renders `value` as `{.field=value,...}` by walking it with
// `__builtin_dump_struct`.
//
// NOTE: Used for classes with private members or with members split between a
// base and the derived class. Non-aggregate members are dispatched through
// `Stringify`; aggregate members are walked inline and bypass it.
template <typename T>
std::string DumpToString(const T& value) {
  StringifySink sink;
  DumpStruct<StringifySink> dump(&sink);
  __builtin_dump_struct(&value, dump);

  return std::move(sink).out();
}

}  // namespace pulse::reflect::internal

namespace pulse {

// Renders any class type as `Type{.field=value,...}`.
//
// NOTE: An explicit `Stringify<T>` always wins over this partial
// specialization.
template <reflect::internal::AutoStringifiable T>
struct Stringify<T> {
  static std::string ToString(const T& value) {
    if constexpr (reflect::internal::Decomposable<T>) {
      return internal::TypeLabel<T>() +
             reflect::internal::MembersToString(value);
    } else {
      return internal::TypeLabel<T>() + reflect::internal::DumpToString(value);
    }
  }

  static std::string DebugString(const T& value) { return ToString(value); }
};

}  // namespace pulse

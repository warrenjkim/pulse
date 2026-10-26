#pragma once

#ifndef PULSE_REFLECT_PLUGIN
#error "pulse/reflect requires the pulse_reflect plugin"
#endif

#include <concepts>
#include <cstddef>
#include <string_view>

namespace pulse::reflect {

// A field descriptor for class `T`. The pulse-reflect plugin generates one per
// field:
//
//   struct PulseField0 {
//     using Type = decltype(::ns::A::i);
//     static consteval const char* Name() { return "i"; }
//     static constexpr const Type& Value(const ::ns::A& v) { return v.i; }
//   };
template <typename D, typename T>
concept FieldDescriptor = requires(const T& value) {
  typename D::Type;
  { D::Name() } -> std::same_as<const char*>;
  { D::Value(value) } -> std::same_as<const typename D::Type&>;
};

// Whether `T` has a `PulseReflect` overload, i.e. is marked with
// `[[pulse::reflect]]`.
template <typename T>
concept Reflected = requires { PulseReflect(static_cast<const T*>(nullptr)); };

// The `Mirror` of `T`.
template <Reflected T>
using Describe = decltype(PulseReflect(static_cast<const T*>(nullptr)));

// The number of fields `T` has.
template <Reflected T>
inline constexpr size_t kFieldCount = Describe<T>::kSize;

// The reflection of class `T`: one descriptor per field, in declaration order.
template <typename T, FieldDescriptor<T>... D>
struct Mirror {
  static constexpr size_t kSize = sizeof...(D);
};

// The name of the field `D` describes.
template <typename D>
inline constexpr std::string_view kFieldName = D::Name();

// Calls `f(name, value)` for each field of `value`, in declaration order.
template <Reflected T, typename F>
constexpr void ForEachField(const T& value, F&& f) {
  [&]<typename... D>(Mirror<T, D...>) {
    (f(kFieldName<D>, D::Value(value)), ...);
  }(Describe<T>{});
}

}  // namespace pulse::reflect

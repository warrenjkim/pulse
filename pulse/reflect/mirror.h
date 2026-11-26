#pragma once

#include <concepts>
#include <cstddef>
#include <string_view>

// Marks a struct or class for reflection:
//
//   struct pulse_reflect A {
//     int i;
//   };
//
// The pulse_reflect plugin generates its reflection into `foo.reflect.h`.
// `foo.h` must include it after its reflected classes.
//
// Reflected classes are printable with `pulse::ToString` from
// `pulse/core/stringify.h`.
#define pulse_reflect [[clang::annotate("pulse::reflect")]]

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
// `pulse_reflect`.
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

namespace internal {

// Names the non-public field of `T` at index `I`.
template <typename T, size_t I>
struct PrivateField {};

// Makes `GetMemberPointer(Tag)` return the member pointer `M`, which may point
// to a private member:
//
//   constexpr auto GetMemberPointer(PrivateField<::ns::A, 1>);
//   template struct MemberPointer<PrivateField<::ns::A, 1>, &::ns::A::i>;
template <typename Tag, auto M>
struct MemberPointer {
  friend constexpr auto GetMemberPointer(Tag) { return M; }
};

template <typename M>
struct MemberTypeOf;

template <typename C, typename T>
struct MemberTypeOf<T C::*> {
  using Type = T;
};

// The type of the member `M` points to, e.g. `const int` for `const int A::*`.
template <typename M>
using MemberType = MemberTypeOf<M>::Type;

// The `Type` and `Value` of the non-public field of `T` at index `I`, whose
// `MemberPointer` is defined. The plugin adds `Name`:
//
//   struct PulseField1 : PrivateFieldDescriptor<::ns::A, 1> {
//     static consteval const char* Name() { return "i"; }
//   };
template <typename T, size_t I>
struct PrivateFieldDescriptor {
  using Type = MemberType<decltype(GetMemberPointer(PrivateField<T, I>{}))>;

  static constexpr const Type& Value(const T& v) {
    return v.*GetMemberPointer(PrivateField<T, I>{});
  }
};

}  // namespace internal

}  // namespace pulse::reflect

#pragma once

namespace pulse {

// A compile-time list of types. Nesting is allowed.
//
// Usage:
//
//   using Numbers = TypeList<int, float>;
//   using All = TypeList<Numbers, std::string>;
template <typename... T>
struct TypeList {};

// Recursively flattens a sequence of types and nested `TypeList`s into a
// single, flat `TypeList`. Non-`TypeList` entries are treated as leaves and
// appended as-is, in order.
//
// Usage:
//
//   using A = TypeList<int, char>;
//   using B = TypeList<A, double, TypeList<float>>;
//
//   Flatten<TypeList<>, B>::Type;  // TypeList<int, char, double, float>
//
// Note: `Flatten` does not validate leaf types. Callers that require leaves
// to satisfy some constraint (e.g. a concept) should enforce it separately,
// after flattening.
template <typename Accumulator, typename... Rest>
struct Flatten;

template <typename... Accumulator>
struct Flatten<TypeList<Accumulator...>> {
  using Type = TypeList<Accumulator...>;
};

template <typename... Accumulator, typename Leaf, typename... Rest>
struct Flatten<TypeList<Accumulator...>, Leaf, Rest...> {
  using Type = typename Flatten<TypeList<Accumulator..., Leaf>, Rest...>::Type;
};

template <typename... Accumulator, typename... Nested, typename... Rest>
struct Flatten<TypeList<Accumulator...>, TypeList<Nested...>, Rest...> {
  using Type =
      typename Flatten<TypeList<Accumulator...>, Nested..., Rest...>::Type;
};

}  // namespace pulse

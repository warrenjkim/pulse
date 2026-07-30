#pragma once

#include <string_view>
#include <tuple>

namespace pulse::reflect {

// A type satisfies `Reflectable` if it exposes a static `Schema()` method
// describing its fields. Example:
//
//   struct Struct {
//     std::string str;
//     double dbl;
//
//     static constexpr auto Schema() {
//       return pulse::reflect::Schema<Struct>{}
//           .Field("str", &Struct::str)
//           .Field("dbl", &Struct::dbl);
//     }
//   };
template <typename StructType>
concept Reflectable = requires { StructType::Schema(); };

// A single named field of `StructType`, described by a member pointer.
// Produced by `Schema::Field()`; consumed via `Schema::ForEachField()` and
// `Schema::Map()`.
//
// TODO(bind nested fields/values)
template <typename StructType, typename FieldType>
struct SchemaField {
  using MemberFieldType = FieldType;

  std::string_view key;
  FieldType StructType::*member;
};

// Describes the fields of `StructType` as an ordered list of `SchemaField`s.
// Fields are registered via `Schema::Field()`, which accepts a name and a
// pointer to the struct member. `Schema` should not be constructed directly
// outside of chaining `Field()` calls.
//
// Fields are visited, in registration order, via `ForEachField()` or
// `Map()`.
template <typename StructType, typename... Fields>
class Schema {
 public:
  explicit Schema() = default;

  // Returns a new `Schema` with an additional field named `name`, bound to
  // `member`. Does not mutate `*this`.
  template <typename FieldType>
  constexpr auto Field(std::string_view name, FieldType StructType::*member) {
    return Schema<StructType, Fields..., SchemaField<StructType, FieldType>>(
        std::tuple_cat(fields_, std::tuple<SchemaField<StructType, FieldType>>{
                                    {.key = name, .member = member}}));
  }

  // Invokes `fn(field)` for each registered `SchemaField`, in registration
  // order. Intended for side-effecting visits (e.g. binding into a result);
  // discards `fn`'s return value. See `Map()` to collect results instead.
  template <typename Fn>
  constexpr void ForEachField(Fn&& fn) const {
    std::apply([&fn](const auto&... fields) { (fn(fields), ...); }, fields_);
  }

  // Invokes `fn(field)` for each registered `SchemaField`, in registration
  // order, and returns the results as a `std::tuple<decltype(fn(fields))...>`.
  // Intended for transforming fields at compile-time.
  template <typename Fn>
  constexpr auto Map(Fn&& fn) const {
    return std::apply(
        [&fn](const auto&... fields) { return std::make_tuple(fn(fields)...); },
        fields_);
  }

 private:
  template <typename, typename...>
  friend class Schema;

  constexpr explicit Schema(std::tuple<Fields...> fields)
      : fields_(std::move(fields)) {}

  std::tuple<Fields...> fields_;
};

}  // namespace pulse::reflect

#pragma once

#include <concepts>
#include <optional>
#include <string_view>
#include <tuple>

#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/json/value.h"
#include "pulse/strings/cat.h"

namespace pulse::json {

// A type satisfies `Bindable` if it defines a `Schema` describing how to
// deserialize a `pulse::json::value` into that type. Example:
//
//   struct MyStruct {
//     std::string str;
//     double dbl;
//     std::optional<std::string> optional_str;
//
//     static auto schema() {
//       return pulse::json::Schema<MyStruct>{}
//           .field("string", &MyStruct::str)
//           .field("double", &MyStruct::dbl)
//           .field("optional_string", &MyStruct::optional_str);
//     }
//   };
template <typename StructType>
concept Bindable = requires { StructType::schema(); };

// Deserializes a `json::value` into `StructType` using the schema defined
// by StructType::schema(). Returns an error if:
//   - `input` is not a JSON object
//   - A required field is missing
//   - A field value has the wrong type
//
// NOTE: Extra fields in the JSON object are ignored.
template <Bindable StructType>
Result<StructType> Bind(Value input);

namespace internal {

// TODO(bind nested fields/values)
template <typename StructType, typename FieldType>
struct Field {
  std::string_view key;
  FieldType StructType::*member;
};

}  // namespace internal

// Describes the mapping from a `json::value` object to a C++ struct. Fields are
// registered via `Schema::Field()`, which accepts a JSON key name and a
// pointer-to-member. Required vs. optional is inferred from the member type.
// `std::optional<T>` members are optional, all others are required.
//
// `Schema` should not be constructed directly. Use `Schema<T>{}.Field(...)` to
// build one. It is intended to be returned from a `static schema()` method on a
// `Bindable` type.
template <typename StructType, typename... Fields>
class Schema {
 public:
  explicit Schema() = default;

  template <typename FieldType>
  constexpr Schema<StructType, Fields...,
                   internal::Field<StructType, FieldType>>
  Field(std::string_view name, FieldType StructType::*member) {
    return Schema<StructType, Fields...,
                  internal::Field<StructType, FieldType>>(std::tuple_cat(
        fields_, std::tuple<internal::Field<StructType, FieldType>>{
                     {.key = name, .member = member}}));
  }

 private:
  template <typename, typename...>
  friend class Schema;

  template <Bindable T>
  friend Result<T> Bind(Value input);

  constexpr explicit Schema(std::tuple<Fields...> fields)
      : fields_(std::move(fields)) {}

  std::tuple<Fields...> fields_;
};

// Implementation details below;

namespace internal {

template <typename T>
concept Optional = requires { typename T::value_type; } &&
                   std::same_as<T, std::optional<typename T::value_type>>;

template <typename StructType, typename FieldType>
Result<void> BindField(const Object& object, StructType* result,
                       const Field<StructType, FieldType>& field) {
  if (auto it = object.find(field.key); it != object.end()) {
    if constexpr (Optional<FieldType>) {
      using ValueType = typename FieldType::value_type;
      if (!it->second.template is<ValueType>()) {
        return Error{.code = Error::Code::kInvalidArgument,
                     .message = strings::Cat("field '", field.key,
                                             "' has the wrong type")};
      }

      result->*(field.member) = it->second.template as<ValueType>();
    } else {
      if (!it->second.template is<FieldType>()) {
        return Error{.code = Error::Code::kInvalidArgument,
                     .message = strings::Cat("field '", field.key,
                                             "' has the wrong type")};
      }

      result->*(field.member) = it->second.template as<FieldType>();
    }

    return Result<void>{};
  }

  if constexpr (Optional<FieldType>) {
    return Result<void>{};
  }

  return Error{
      .code = Error::Code::kInvalidArgument,
      .message = strings::Cat("missing required field '", field.key, "'")};
}

}  // namespace internal

template <Bindable StructType>
Result<StructType> Bind(Value input) {
  if (!input.is<Object>()) {
    return Error{.code = Error::Code::kInvalidArgument,
                 .message = "expected object"};
  }

  const Object& object = input.as<Object>();
  StructType result{};
  Result<void> err;
  if (!std::apply(
          [&object, &result, &err](const auto&... fields) {
            return ((err = internal::BindField(object, &result, fields),
                     err.ok()) &&
                    ...);
          },
          StructType::schema().fields_)) {
    return err.error();
  }

  return result;
}

}  // namespace pulse::json

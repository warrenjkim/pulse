#pragma once

#include <concepts>
#include <optional>

#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/json/value.h"
#include "pulse/reflect/schema.h"
#include "pulse/strings/cat.h"

namespace pulse::json {

// A type satisfies `Bindable` if it defines a `Schema()` describing how to
// deserialize a `pulse::json::Value` into that type. Example:
//
//   struct Struct {
//     std::string str;
//     double dbl;
//     std::optional<std::string> optional_str;
//
//     static constexpr auto Schema() {
//       return pulse::reflect::Schema<Struct>{}
//           .Field("string", &Struct::str)
//           .Field("double", &Struct::dbl)
//           .Field("optional_string", &Struct::optional_str);
//     }
//   };
template <typename StructType>
concept Bindable = reflect::Reflectable<StructType>;

// Deserializes a `json::Value` into `StructType` using the schema defined
// by StructType::Schema(). Returns an error if:
//   - `input` is not a JSON object
//   - A required field is missing
//   - A field value has the wrong type
//
// NOTE: Extra fields in the JSON object are ignored.
template <Bindable StructType>
Result<StructType> Bind(Value input);

// Implementation details below;

namespace internal {

template <typename T>
concept Optional = requires { typename T::value_type; } &&
                   std::same_as<T, std::optional<typename T::value_type>>;

template <typename StructType, typename FieldType>
Result<void> BindField(
    const Object& object, StructType* result,
    const reflect::SchemaField<StructType, FieldType>& field) {
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
  StructType::Schema().ForEachField(
      [&object, &result, &err](const auto& field) {
        if (!err.ok()) {
          return;
        }

        err = internal::BindField(object, &result, field);
      });

  if (!err.ok()) {
    return err.error();
  }

  return result;
}

}  // namespace pulse::json

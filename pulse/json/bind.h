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

template <typename StructType>
concept Bindable = requires { StructType::schema(); };

template <Bindable StructType>
Result<StructType> bind(value input);

namespace internal {

// TODO(bind nested fields/values)
template <typename StructType, typename FieldType>
struct Field {
  std::string_view key;
  FieldType StructType::*member;
};

}  // namespace internal

template <typename StructType, typename... Fields>
class Schema {
 public:
  explicit Schema() = default;

  template <typename FieldType>
  constexpr Schema<StructType, Fields...,
                   internal::Field<StructType, FieldType>>
  field(std::string_view name, FieldType StructType::*member) {
    return Schema<StructType, Fields...,
                  internal::Field<StructType, FieldType>>(std::tuple_cat(
        fields_, std::tuple<internal::Field<StructType, FieldType>>{
                     {.key = name, .member = member}}));
  }

 private:
  template <typename, typename...>
  friend class Schema;

  template <Bindable T>
  friend Result<T> bind(value input);

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
Result<void> bind_field(const object_t& object, StructType* result,
                        const Field<StructType, FieldType>& field) {
  if (auto it = object.find(field.key); it != object.end()) {
    if constexpr (Optional<FieldType>) {
      using ValueType = typename FieldType::value_type;
      if (!it->second.template is<ValueType>()) {
        return Error{.code = Error::Code::kInvalidArgument,
                     .message = strings::cat("field '", field.key,
                                             "' has the wrong type")};
      }

      result->*(field.member) = it->second.template as<ValueType>();
    } else {
      if (!it->second.template is<FieldType>()) {
        return Error{.code = Error::Code::kInvalidArgument,
                     .message = strings::cat("field '", field.key,
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
      .message = strings::cat("missing required field '", field.key, "'")};
}

}  // namespace internal

template <Bindable StructType>
Result<StructType> bind(value input) {
  if (!input.is<object_t>()) {
    return Error{.code = Error::Code::kInvalidArgument,
                 .message = "expected object"};
  }

  const object_t& object = input.as<object_t>();
  StructType result{};
  Result<void> err;
  if (!std::apply(
          [&object, &result, &err](const auto&... fields) {
            return ((err = internal::bind_field(object, &result, fields),
                     err.ok()) &&
                    ...);
          },
          StructType::schema().fields_)) {
    return err.error();
  }

  return result;
}

}  // namespace pulse::json

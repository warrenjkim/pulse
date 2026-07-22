#pragma once

#include <string_view>
#include <tuple>

namespace pulse::reflect {

template <typename StructType>
concept Reflectable = requires { StructType::Schema(); };

// TODO(bind nested fields/values)
template <typename StructType, typename FieldType>
struct SchemaField {
  std::string_view key;
  FieldType StructType::*member;
};

template <typename StructType, typename... Fields>
class Schema {
 public:
  explicit Schema() = default;

  template <typename FieldType>
  constexpr auto Field(std::string_view name, FieldType StructType::*member) {
    return Schema<StructType, Fields..., SchemaField<StructType, FieldType>>(
        std::tuple_cat(fields_, std::tuple<SchemaField<StructType, FieldType>>{
                                    {.key = name, .member = member}}));
  }

  template <typename Fn>
  constexpr void ForEachField(Fn&& fn) const {
    std::apply([&fn](const auto&... fields) { (fn(fields), ...); }, fields_);
  }

 private:
  template <typename, typename...>
  friend class Schema;

  constexpr explicit Schema(std::tuple<Fields...> fields)
      : fields_(std::move(fields)) {}

  std::tuple<Fields...> fields_;
};

}  // namespace pulse::reflect

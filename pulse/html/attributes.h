#pragma once

#include <concepts>
#include <string>
#include <string_view>
#include <tuple>

#include "pulse/core/type_list.h"
#include "pulse/strings/string_literal.h"

namespace pulse::html {

struct Attribute {
  std::string_view key;
  std::string value;
};

namespace internal {

template <strings::StringLiteral kName>
struct Attribute {
  static constexpr std::string_view kKey = kName;
  std::string value;
  operator ::pulse::html::Attribute() const {
    return ::pulse::html::Attribute{.key = kKey, .value = value};
  }
};

}  // namespace internal

template <typename T>
concept AttributeType = requires {
  { T::kKey } -> std::convertible_to<std::string_view>;
} && std::convertible_to<T, Attribute>;

template <AttributeType... Attrs>
struct Attributes {
  Attributes(Attrs... attrs) : attrs(std::move(attrs)...) {}

  std::tuple<Attrs...> attrs;
};

template <AttributeType... T>
using AttributeList = TypeList<T...>;

template <typename Attr, typename Tag>
concept AttributeAllowed =
    Contains<Attr, typename Tag::AllowedAttributes>::value;

using Class = internal::Attribute<"class">;
using Href = internal::Attribute<"href">;
using Id = internal::Attribute<"id">;
using RowSpan = internal::Attribute<"rowspan">;
using ColSpan = internal::Attribute<"colspan">;
using Action = internal::Attribute<"action">;
using Method = internal::Attribute<"method">;
using Type = internal::Attribute<"type">;
using Name = internal::Attribute<"name">;
using Value = internal::Attribute<"value">;
using Placeholder = internal::Attribute<"placeholder">;
using For = internal::Attribute<"for">;
using OnClick = internal::Attribute<"onclick">;

}  // namespace pulse::html

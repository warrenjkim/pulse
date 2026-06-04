#pragma once

#include <concepts>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

namespace pulse::html {

struct Attribute {
  std::string_view key;
  std::string value;
};

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
struct AttributeList {};

template <typename Tag, typename List>
struct Contains {};

template <typename T, typename... List>
struct Contains<T, AttributeList<List...>>
    : std::bool_constant<(std::same_as<T, List> || ...)> {};

template <typename Attr, typename Tag>
concept AttributeAllowed =
    Contains<Attr, typename Tag::AllowedAttributes>::value;

struct Class {
  static constexpr std::string_view kKey = "class";
  std::string value;

  operator Attribute() const { return Attribute{.key = kKey, .value = value}; }
};

struct Href {
  static constexpr std::string_view kKey = "href";
  std::string value;

  operator Attribute() const { return Attribute{.key = kKey, .value = value}; }
};

struct Id {
  static constexpr std::string_view kKey = "id";
  std::string value;

  operator Attribute() const { return Attribute{.key = kKey, .value = value}; }
};

struct RowSpan {
  static constexpr std::string_view kKey = "rowspan";
  std::string value;

  operator Attribute() const { return Attribute{.key = kKey, .value = value}; }
};

struct ColSpan {
  static constexpr std::string_view kKey = "colspan";
  std::string value;

  operator Attribute() const { return Attribute{.key = kKey, .value = value}; }
};

}  // namespace pulse::html

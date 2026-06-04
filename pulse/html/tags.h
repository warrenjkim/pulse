#pragma once

#include <concepts>
#include <string_view>

#include "pulse/html/attributes.h"

namespace pulse::html {

template <typename T>
concept Tag = requires {
  { T::kTag } -> std::convertible_to<std::string_view>;
  typename T::AllowedAttributes;
};

struct Div {
  static constexpr std::string_view kTag = "div";
  using AllowedAttributes = Attributes<Class, Id>;
};

struct H1 {
  static constexpr std::string_view kTag = "h1";
  using AllowedAttributes = Attributes<Class, Id>;
};

struct H2 {
  static constexpr std::string_view kTag = "h2";
  using AllowedAttributes = Attributes<Class, Id>;
};

struct H3 {
  static constexpr std::string_view kTag = "h3";
  using AllowedAttributes = Attributes<Class, Id>;
};

struct P {
  static constexpr std::string_view kTag = "p";
  using AllowedAttributes = Attributes<Class, Id>;
};

struct Span {
  static constexpr std::string_view kTag = "span";
  using AllowedAttributes = Attributes<Class, Id>;
};

struct Table {
  static constexpr std::string_view kTag = "table";
  using AllowedAttributes = Attributes<Class, Id>;
};

struct Thead {
  static constexpr std::string_view kTag = "thead";
  using AllowedAttributes = Attributes<Class, Id>;
};

struct Tbody {
  static constexpr std::string_view kTag = "tbody";
  using AllowedAttributes = Attributes<Class, Id>;
};

struct Tr {
  static constexpr std::string_view kTag = "tr";
  using AllowedAttributes = Attributes<Class, Id>;
};

struct Th {
  static constexpr std::string_view kTag = "th";
  using AllowedAttributes = Attributes<Class, Id, RowSpan, ColSpan>;
};

struct Td {
  static constexpr std::string_view kTag = "td";
  using AllowedAttributes = Attributes<Class, Id, RowSpan, ColSpan>;
};

struct A {
  static constexpr std::string_view kTag = "a";
  using AllowedAttributes = Attributes<Class, Id, Href>;
};

struct Html {
  static constexpr std::string_view kTag = "html";
  using AllowedAttributes = Attributes<>;
};

struct Head {
  static constexpr std::string_view kTag = "head";
  using AllowedAttributes = Attributes<>;
};

struct Body {
  static constexpr std::string_view kTag = "body";
  using AllowedAttributes = Attributes<Class, Id>;
};

struct Title {
  static constexpr std::string_view kTag = "title";
  using AllowedAttributes = Attributes<>;
};

}  // namespace pulse::html

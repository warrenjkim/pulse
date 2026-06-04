#pragma once

#include <concepts>
#include <string_view>

#include "pulse/html/attributes.h"

namespace pulse::html {

template <typename T>
concept TagType = requires {
  { T::kTag } -> std::convertible_to<std::string_view>;
  typename T::AllowedAttributes;
};

struct Div {
  static constexpr std::string_view kTag = "div";
  using AllowedAttributes = AttributeList<Class, Id>;
};

struct H1 {
  static constexpr std::string_view kTag = "h1";
  using AllowedAttributes = AttributeList<Class, Id>;
};

struct H2 {
  static constexpr std::string_view kTag = "h2";
  using AllowedAttributes = AttributeList<Class, Id>;
};

struct H3 {
  static constexpr std::string_view kTag = "h3";
  using AllowedAttributes = AttributeList<Class, Id>;
};

struct P {
  static constexpr std::string_view kTag = "p";
  using AllowedAttributes = AttributeList<Class, Id>;
};

struct Span {
  static constexpr std::string_view kTag = "span";
  using AllowedAttributes = AttributeList<Class, Id>;
};

struct Table {
  static constexpr std::string_view kTag = "table";
  using AllowedAttributes = AttributeList<Class, Id>;
};

struct Thead {
  static constexpr std::string_view kTag = "thead";
  using AllowedAttributes = AttributeList<Class, Id>;
};

struct Tbody {
  static constexpr std::string_view kTag = "tbody";
  using AllowedAttributes = AttributeList<Class, Id>;
};

struct Tr {
  static constexpr std::string_view kTag = "tr";
  using AllowedAttributes = AttributeList<Class, Id>;
};

struct Th {
  static constexpr std::string_view kTag = "th";
  using AllowedAttributes = AttributeList<Class, Id, RowSpan, ColSpan>;
};

struct Td {
  static constexpr std::string_view kTag = "td";
  using AllowedAttributes = AttributeList<Class, Id, RowSpan, ColSpan>;
};

struct A {
  static constexpr std::string_view kTag = "a";
  using AllowedAttributes = AttributeList<Class, Id, Href>;
};

struct Html {
  static constexpr std::string_view kTag = "html";
  using AllowedAttributes = AttributeList<>;
};

struct Head {
  static constexpr std::string_view kTag = "head";
  using AllowedAttributes = AttributeList<>;
};

struct Body {
  static constexpr std::string_view kTag = "body";
  using AllowedAttributes = AttributeList<Class, Id>;
};

struct Title {
  static constexpr std::string_view kTag = "title";
  using AllowedAttributes = AttributeList<>;
};

}  // namespace pulse::html

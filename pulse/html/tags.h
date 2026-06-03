#pragma once

#include <concepts>
#include <string_view>

namespace pulse::html {

template <typename T>
concept Tag = requires {
  { T::kTag } -> std::convertible_to<std::string_view>;
};

// begin layout

struct Div {
  static constexpr std::string_view kTag = "div";
};

// end layout

// begin typography

struct H1 {
  static constexpr std::string_view kTag = "h1";
};

struct H2 {
  static constexpr std::string_view kTag = "h2";
};

struct H3 {
  static constexpr std::string_view kTag = "h3";
};

struct P {
  static constexpr std::string_view kTag = "p";
};

struct Span {
  static constexpr std::string_view kTag = "span";
};

// end typography

// begin tables
struct Table {
  static constexpr std::string_view kTag = "table";
};

struct Thead {
  static constexpr std::string_view kTag = "thead";
};

struct Tbody {
  static constexpr std::string_view kTag = "tbody";
};

struct Tr {
  static constexpr std::string_view kTag = "tr";
};

struct Th {
  static constexpr std::string_view kTag = "th";
};

struct Td {
  static constexpr std::string_view kTag = "td";
};

// end tables

// begin links

struct A {
  static constexpr std::string_view kTag = "a";
};

// end links

// begin structure

struct Html {
  static constexpr std::string_view kTag = "html";
};

struct Head {
  static constexpr std::string_view kTag = "head";
};

struct Body {
  static constexpr std::string_view kTag = "body";
};

struct Title {
  static constexpr std::string_view kTag = "title";
};

// end structure

}  // namespace pulse::html

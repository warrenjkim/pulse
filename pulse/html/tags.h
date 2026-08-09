#pragma once

#include <concepts>
#include <string_view>

#include "pulse/html/attributes.h"
#include "pulse/strings/string_literal.h"

namespace pulse::html {

template <strings::StringLiteral kName, typename Attrs = AttributeList<>>
struct Tag {
  static constexpr std::string_view kTag = kName;
  using AllowedAttributes = Attrs;
};

template <typename T>
concept TagType = requires {
  { T::kTag } -> std::convertible_to<std::string_view>;
  typename T::AllowedAttributes;
};

using A = Tag<"a", AttributeList<Class, Id, Href>>;
using Body = Tag<"body", AttributeList<Class, Id>>;
using Button = Tag<"button", AttributeList<Class, Id, Type, OnClick>>;
using Div = Tag<"div", AttributeList<Class, Id>>;
using Form = Tag<"form", AttributeList<Class, Id, Action, Method>>;
using H1 = Tag<"h1", AttributeList<Class, Id>>;
using H2 = Tag<"h2", AttributeList<Class, Id>>;
using H3 = Tag<"h3", AttributeList<Class, Id>>;
using Head = Tag<"head">;
using Hr = Tag<"hr">;
using Html = Tag<"html">;
using Input =
    Tag<"input", AttributeList<Class, Id, Type, Name, Value, Placeholder>>;
using Label = Tag<"label", AttributeList<Class, Id, For>>;
using Nav = Tag<"nav", AttributeList<Class, Id>>;
using Option = Tag<"option", AttributeList<Value>>;
using P = Tag<"p", AttributeList<Class, Id>>;
using Pre = Tag<"pre", AttributeList<Class, Id>>;
using Script = Tag<"script">;
using Select = Tag<"select", AttributeList<Class, Id, Name>>;
using Span = Tag<"span", AttributeList<Class, Id>>;
using Style = Tag<"style">;
using Table = Tag<"table", AttributeList<Class, Id>>;
using Tbody = Tag<"tbody", AttributeList<Class, Id>>;
using Td = Tag<"td", AttributeList<Class, Id, RowSpan, ColSpan>>;
using Th = Tag<"th", AttributeList<Class, Id, RowSpan, ColSpan>>;
using Thead = Tag<"thead", AttributeList<Class, Id>>;
using Title = Tag<"title">;
using Tr = Tag<"tr", AttributeList<Class, Id>>;

}  // namespace pulse::html

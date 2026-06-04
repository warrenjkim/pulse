#pragma once

#include <concepts>
#include <type_traits>

#include "pulse/html/attributes.h"

namespace pulse::html {

template <typename Tag, typename List>
struct Contains {};

template <typename T, typename... List>
struct Contains<T, Attributes<List...>>
    : std::bool_constant<(std::same_as<T, List> || ...)> {};

template <typename Attr, typename Tag>
concept AttributeAllowed =
    Contains<Attr, typename Tag::AllowedAttributes>::value;

}  // namespace pulse::html

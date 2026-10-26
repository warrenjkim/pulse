#pragma once

// Stands in for `pulse/reflect/mirror.h` in -verify tests, which compile
// without include paths. Generated code needs only this name.
namespace pulse::reflect {

template <typename T, typename... D>
struct Mirror {};

}  // namespace pulse::reflect

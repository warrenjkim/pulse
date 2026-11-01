#pragma once

// Stands in for `pulse/reflect/mirror.h` in -verify tests, which compile
// without include paths.
#define pulse_reflect [[clang::annotate("pulse::reflect")]]

#pragma once

#include "pulse/reflect/mirror.h"

namespace ns {

struct pulse_reflect A {
  int i;
  bool j;
};

struct pulse_reflect B {
  A a;
};

struct C {};

struct pulse_reflect D {
  C c;
};

}  // namespace ns

#include "pulse/reflect/testdata/types.reflect.h"

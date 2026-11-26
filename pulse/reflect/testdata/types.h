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

class pulse_reflect E {
 public:
  explicit E(int i) : i_(i) {}

 private:
  int i_;
};

}  // namespace ns

#include "pulse/reflect/testdata/types.reflect.h"

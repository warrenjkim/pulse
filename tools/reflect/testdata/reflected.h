#pragma once

#include "pulse/reflect/mirror.h"

namespace ns {

struct pulse_reflect A {
  int i;
  bool j;
};

class pulse_reflect B {
 public:
  constexpr B(int i, int j, int k) : i(i), j_(j), k_(k) {}

  int i;

 protected:
  int j_;

 private:
  const int k_;
};

}  // namespace ns

#include "tools/reflect/testdata/reflected.reflect.h"

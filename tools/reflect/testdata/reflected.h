#pragma once

namespace ns {

struct [[pulse::reflect]] A {
  int i;
  bool j;
};

}  // namespace ns

#include "tools/reflect/testdata/reflected.reflect.h"

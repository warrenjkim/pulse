// expected-error@+1 {{include "testdata/empty.reflect.h" after the last reflected class}}
#include "testdata/empty.reflect.h"

struct pulse_reflect A {
  int i;
};

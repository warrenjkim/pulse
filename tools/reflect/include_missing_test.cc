struct pulse_reflect A {
  int i;
  // expected-error@+1 {{does not include its generated source; add `#include "testdata/empty.reflect.h"`}}
};

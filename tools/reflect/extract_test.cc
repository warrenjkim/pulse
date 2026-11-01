struct pulse_reflect Struct {
  int i;
  int j;
};

struct pulse_reflect WithoutFields {};

namespace inner {

struct pulse_reflect InNamespace {
  int i;
};

}  // namespace inner

namespace {

struct pulse_reflect InAnonymousNamespace {
  int i;
};

}  // namespace

struct pulse_reflect WithUnnamedUnionMember {
  union {
    int i;
    float f;
  } u;
};

struct pulse_reflect WithNamedUnionMember {
  union V {
    int i;
    float f;
  };

  V v;
};

// expected-error@+1 {{unnamed classes cannot be reflected}}
struct pulse_reflect {
  int i;
} unnamed [[maybe_unused]];

void Local() {
  // expected-error@+1 {{local classes cannot be reflected}}
  struct pulse_reflect L {
    int i;
  };
}

struct Outer {
  // expected-error@+1 {{nested classes are not supported yet}}
  struct pulse_reflect Nested {
    int i;
  };
};

// expected-error@+2 {{class templates are not supported yet}}
template <typename T>
struct pulse_reflect Template {
  T v;
};

Template<int> instance;

struct Base {
  int i;
};

// expected-error@+1 {{base classes are not supported yet}}
struct pulse_reflect WithBase : Base {
  int j;
};

class pulse_reflect Class {
  // expected-error@+1 {{non-public fields are not supported yet}}
  int i [[maybe_unused]];
};

struct pulse_reflect WithBitfield {
  // expected-error@+1 {{bitfields are not supported yet}}
  int i : 1;
};

struct pulse_reflect WithAnonymousUnion {
  // expected-error@+1 {{anonymous unions cannot be reflected}}
  union {
    int i;
    float f;
  };
};

struct pulse_reflect WithDeclarator {
  int i;
  // expected-error@+1 {{declare variables separately from a reflected class}}
} declarator;

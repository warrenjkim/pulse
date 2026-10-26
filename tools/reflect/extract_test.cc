struct [[pulse::reflect]] Struct {
  int i;
  int j;
};

struct [[pulse::reflect]] WithoutFields {};

namespace inner {

struct [[pulse::reflect]] InNamespace {
  int i;
};

}  // namespace inner

namespace {

struct [[pulse::reflect]] InAnonymousNamespace {
  int i;
};

}  // namespace

struct [[pulse::reflect]] WithUnnamedUnionMember {
  union {
    int i;
    float f;
  } u;
};

struct [[pulse::reflect]] WithNamedUnionMember {
  union V {
    int i;
    float f;
  };

  V v;
};

// expected-error@+1 {{unnamed classes cannot be reflected}}
struct [[pulse::reflect]] {
  int i;
} unnamed [[maybe_unused]];

void Local() {
  // expected-error@+1 {{local classes cannot be reflected}}
  struct [[pulse::reflect]] L {
    int i;
  };
}

struct Outer {
  // expected-error@+1 {{nested classes are not supported yet}}
  struct [[pulse::reflect]] Nested {
    int i;
  };
};

// expected-error@+2 {{class templates are not supported yet}}
template <typename T>
struct [[pulse::reflect]] Template {
  T v;
};

Template<int> instance;

struct Base {
  int i;
};

// expected-error@+1 {{base classes are not supported yet}}
struct [[pulse::reflect]] WithBase : Base {
  int j;
};

class [[pulse::reflect]] Class {
  // expected-error@+1 {{non-public fields are not supported yet}}
  int i [[maybe_unused]];
};

struct [[pulse::reflect]] WithBitfield {
  // expected-error@+1 {{bitfields are not supported yet}}
  int i : 1;
};

struct [[pulse::reflect]] WithAnonymousUnion {
  // expected-error@+1 {{anonymous unions cannot be reflected}}
  union {
    int i;
    float f;
  };
};

struct [[pulse::reflect]] WithDeclarator {
  int i;
  // expected-error@+1 {{declare variables separately from a reflected class}}
} declarator;

struct [[pulse::reflect]] Struct {
  int i;
};

class [[pulse::reflect]] Class {
  int i [[maybe_unused]];
};

struct Base {
  int i;
};

struct [[pulse::reflect]] WithBase : Base {
  int j;
};

template <typename T>
struct [[pulse::reflect]] Template {
  T v;
};

struct Outer {
  struct [[pulse::reflect]] Nested {
    int i;
  };
};

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

struct [[pulse::reflect]] WithoutFields {
  using T = int;
  static constexpr bool kB = true;
};

struct [[pulse::reflect]] WithAnonymousUnion {
  unsigned i;
  union {
    char s[8];
    const char* p;
  };
};

struct [[pulse::reflect]] WithAnonymousUnions {
  union {
    int i;
    float f;
  };
  union {
    long j;
    double d;
  };
};

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

// expected-error@+1 {{only applies to structs and classes}}
union [[pulse::reflect]] Union {
  int i;
  float f;
};

// expected-error@+1 {{only applies to structs and classes}}
enum class [[pulse::reflect]] Enum { kA };

// expected-error@+1 {{only applies to structs and classes}}
[[pulse::reflect]] void Function();

// expected-error@+1 {{only applies to structs and classes}}
[[pulse::reflect]] int variable;

struct Members {
  // expected-error@+1 {{only applies to structs and classes}}
  [[pulse::reflect]] int field;

  // expected-error@+1 {{only applies to structs and classes}}
  [[pulse::reflect]] void Method();
};

// expected-error@+1 {{only applies to structs and classes}}
namespace [[pulse::reflect]] reflected {}

struct [[pulse::reflect(1)]] WithArguments {
  int i;
};

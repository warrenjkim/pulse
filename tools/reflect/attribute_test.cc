struct [[pulse::reflect]] Struct {
  int i;
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

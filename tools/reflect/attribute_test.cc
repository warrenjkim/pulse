struct pulse_reflect Struct {
  int i;
};

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

struct pulse_reflect WithoutFields {
  using T = int;
  static constexpr bool kB = true;
};

// expected-error@+1 {{only applies to structs and classes}}
union pulse_reflect Union {
  int i;
  float f;
};

// expected-error@+1 {{only applies to structs and classes}}
enum class pulse_reflect Enum { kA };

// expected-error@+1 {{only applies to structs and classes}}
pulse_reflect void Function();

// expected-error@+1 {{only applies to structs and classes}}
pulse_reflect int variable;

struct Members {
  // expected-error@+1 {{only applies to structs and classes}}
  pulse_reflect int field;

  // expected-error@+1 {{only applies to structs and classes}}
  pulse_reflect void Method();
};

// expected-error@+1 {{only applies to structs and classes}}
namespace pulse_reflect reflected {}

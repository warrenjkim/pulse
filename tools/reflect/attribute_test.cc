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

// expected-error@+1 {{unions cannot be reflected}}
union pulse_reflect Union {
  int i;
  float f;
};

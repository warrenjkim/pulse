// expected-no-diagnostics

struct [[pulse::reflect]] A {
  int i;
};

static_assert(PulseInjected(static_cast<const A*>(nullptr)));

namespace ns {

struct [[pulse::reflect]] B {
  int i;
};

}  // namespace ns

static_assert(PulseInjected(static_cast<const ns::B*>(nullptr)));

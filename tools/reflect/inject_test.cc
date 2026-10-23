// expected-no-diagnostics

template <typename M>
struct Size;

template <typename T, typename... D>
struct Size<pulse::reflect::Mirror<T, D...>> {
  static constexpr int kValue = sizeof...(D);
};

template <typename T>
constexpr int kSize =
    Size<decltype(PulseReflect(static_cast<const T*>(nullptr)))>::kValue;

struct [[pulse::reflect]] A {
  int i;
};

namespace ns {

struct [[pulse::reflect]] B {
  int i;
  int j;
};

}  // namespace ns

namespace {

struct [[pulse::reflect]] C {};

}  // namespace

static_assert(kSize<A> == 1);
static_assert(kSize<ns::B> == 2);
static_assert(kSize<C> == 0);

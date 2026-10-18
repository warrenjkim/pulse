#include "tools/reflect/render.h"

#include "gtest/gtest.h"
#include "tools/reflect/record.h"

namespace pulse::reflect::plugin {

namespace {

TEST(RenderTest, Empty) {
  EXPECT_EQ(Render(Record{.qualified_name = "A"}),
            R"([[maybe_unused]] consteval auto PulseReflect(const ::A*) {
  return ::pulse::reflect::Mirror<::A>{};
}
)");
}

TEST(RenderTest, Fields) {
  EXPECT_EQ(
      Render(Record{.qualified_name = "ns::A",
                    .fields = {Record::Field{.name = "i"}, {.name = "j"}}}),
      R"([[maybe_unused]] consteval auto PulseReflect(const ::ns::A*) {
  struct PulseField0 {
    using Type = decltype(::ns::A::i);
    static consteval const char* Name() { return "i"; }
    static constexpr const Type& Value(const ::ns::A& v) { return v.i; }
  };
  struct PulseField1 {
    using Type = decltype(::ns::A::j);
    static consteval const char* Name() { return "j"; }
    static constexpr const Type& Value(const ::ns::A& v) { return v.j; }
  };
  return ::pulse::reflect::Mirror<::ns::A, PulseField0, PulseField1>{};
}
)");
}

}  // namespace

}  // namespace pulse::reflect::plugin

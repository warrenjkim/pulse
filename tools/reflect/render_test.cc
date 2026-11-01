#include "tools/reflect/render.h"

#include "gtest/gtest.h"
#include "tools/reflect/record.h"

namespace pulse::reflect::plugin {

namespace {

TEST(RenderTest, Empty) {
  EXPECT_EQ(RenderRecord(Record{.qualified_name = "A"}),
            R"([[maybe_unused]] consteval auto PulseReflect(const ::A*) {
  return ::pulse::reflect::Mirror<::A>{};
}
)");
}

TEST(RenderTest, Fields) {
  EXPECT_EQ(RenderRecord(
                Record{.qualified_name = "ns::A",
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

TEST(RenderTest, EmptyHeader) { EXPECT_EQ(RenderHeader({}), "#pragma once\n"); }

TEST(RenderTest, Header) {
  EXPECT_EQ(
      RenderHeader({Record{.qualified_name = "A"},
                    Record{.qualified_name = "ns::v1::B",
                           .namespaces = {{.name = "ns"},
                                          {.name = "v1", .is_inline = true},
                                          {}}}}),
      R"(#pragma once

#include "pulse/reflect/mirror.h"

[[maybe_unused]] consteval auto PulseReflect(const ::A*) {
  return ::pulse::reflect::Mirror<::A>{};
}

namespace ns {
inline namespace v1 {
namespace {
[[maybe_unused]] consteval auto PulseReflect(const ::ns::v1::B*) {
  return ::pulse::reflect::Mirror<::ns::v1::B>{};
}
}  // namespace
}  // namespace v1
}  // namespace ns
)");
}

}  // namespace

}  // namespace pulse::reflect::plugin

#include "pulse/reflect/dump_struct.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/core/container_stringify.h"  // IWYU pragma: keep
#include "pulse/core/stringify.h"
#include "pulse/reflect/data_sink.h"

namespace pulse::reflect {

namespace {

using ::testing::Eq;
using ::testing::Lt;
using ::testing::Not;

class MetaSink {
 public:
  void BeginStruct() { trace_.push_back("BeginStruct()"); }
  void EndStruct() { trace_.push_back("EndStruct()"); }
  void BeginField(std::string_view type, std::string_view name) {
    trace_.push_back("BeginField(" + std::string(type) + "," +
                     std::string(name) + ")");
  }

  template <Stringifiable T>
  void Field(std::string_view name, const T& value) {
    trace_.push_back("Field(" + std::string(name) + "," +
                     pulse::ToString(value) + ")");
  }

  const std::vector<std::string>& trace() const { return trace_; }

 private:
  std::vector<std::string> trace_;
};
static_assert(DataSink<MetaSink>);

TEST(DumpStructTest, RejectsNonGenericSinks) {
  class ConcreteSink {
   public:
    void BeginStruct() {}
    void EndStruct() {}
    void BeginField(std::string_view, std::string_view) {}
    void Field(std::string_view, int) {}
    void Field(std::string_view, const std::string&) {}
  };

  static_assert(!DataSink<ConcreteSink>);
}

#if __has_builtin(__builtin_dump_struct)

template <typename T>
std::vector<std::string> Dump(const T& value) {
  MetaSink sink;
  DumpStruct<MetaSink> dump(&sink);
  __builtin_dump_struct(&value, dump);
  return sink.trace();
}

TEST(DumpStructTest, FlatStruct) {
  struct Flat {
    int i;
    bool b;
    std::string s;
  };

  EXPECT_THAT(Dump(Flat{.i = 1, .b = true, .s = "x"}),
              Eq(std::vector<std::string>{
                  "BeginStruct()",
                  "Field(i,1)",
                  "Field(b,true)",
                  "Field(s,\"x\")",
                  "EndStruct()",
              }));
}

TEST(DumpStructTest, EmptyStruct) {
  struct Empty {};

  EXPECT_THAT(Dump(Empty{}), Eq(std::vector<std::string>{
                                 "BeginStruct()",
                                 "EndStruct()",
                             }));
}

TEST(DumpStructTest, NestedStructs) {
  struct C {
    int i;
  };

  struct B {
    C c;
    int j;
  };

  struct A {
    B b;
  };

  EXPECT_THAT(Dump(A{.b = {.c = {.i = 1}, .j = 2}}),
              Eq(std::vector<std::string>{
                  "BeginStruct()",
                  "BeginField(B,b)",
                  "BeginStruct()",
                  "BeginField(C,c)",
                  "BeginStruct()",
                  "Field(i,1)",
                  "EndStruct()",
                  "Field(j,2)",
                  "EndStruct()",
                  "EndStruct()",
              }));
}

TEST(DumpStructTest, MultiLevelInheritance) {
  struct A {
    int i;
  };

  struct B : A {
    int j;
  };

  struct C : B {
    int k;
  };

  EXPECT_THAT(Dump(C{B{A{.i = 1}, /*j=*/2}, /*k=*/3}),
              Eq(std::vector<std::string>{
                  "BeginStruct()",
                  "Field(i,1)",
                  "Field(j,2)",
                  "Field(k,3)",
                  "EndStruct()",
              }));
}

TEST(DumpStructTest, MultipleInheritance) {
  struct A {
    int i;
  };

  struct B {
    int j;
  };

  struct C : A, B {
    int k;
  };

  EXPECT_THAT(Dump(C{A{.i = 1}, B{.j = 2}, /*k=*/3}),
              Eq(std::vector<std::string>{
                  "BeginStruct()",
                  "Field(i,1)",
                  "Field(j,2)",
                  "Field(k,3)",
                  "EndStruct()",
              }));
}

TEST(DumpStructTest, BaseAndFieldOfSameType) {
  struct Base {
    int i;
  };

  struct Derived : Base {
    Base base;
  };

  EXPECT_THAT(Dump(Derived{Base{.i = 1}, /*base=*/{.i = 2}}),
              Eq(std::vector<std::string>{
                  "BeginStruct()",
                  "Field(i,1)",
                  "BeginField(Base,base)",
                  "BeginStruct()",
                  "Field(i,2)",
                  "EndStruct()",
                  "EndStruct()",
              }));
}

TEST(DumpStructTest, BaseWithTemplatedMember) {
  struct Base {
    std::vector<int> v;
  };

  struct Derived : Base {
    int i;
  };

  const std::vector<std::string> trace =
      Dump(Derived{Base{.v = {1, 2, 3}}, /*i=*/4});

  EXPECT_THAT(std::count(trace.begin(), trace.end(), "BeginStruct()"), Eq(1));
  EXPECT_THAT(std::count(trace.begin(), trace.end(), "EndStruct()"), Eq(1));
}

TEST(DumpStructTest, Bitfields) {
  struct Bits {
    int i;
    unsigned int j : 1;
    unsigned int k : 4;
    int l;
  };

  EXPECT_THAT(Dump(Bits{.i = 1, .j = 0, .k = 4, .l = 2}),
              Eq(std::vector<std::string>{
                  "BeginStruct()",
                  "Field(i,1)",
                  "Field(j,0)",
                  "Field(k,4)",
                  "Field(l,2)",
                  "EndStruct()",
              }));
}

TEST(DumpStructTest, ContainerFields) {
  struct Containers {
    std::vector<int> v;
    std::map<std::string, int> m;
    std::vector<std::vector<int>> n;
    std::vector<int> e;
  };

  EXPECT_THAT(
      Dump(Containers{
          .v = {1, 2, 3}, .m = {{"a", 1}}, .n = {{1, 2}, {3}}, .e = {}}),
      Eq(std::vector<std::string>{
          "BeginStruct()",
          "Field(v,std::vector<int>{1,2,3})",
          R"(Field(m,std::map<std::string,int>{{"a",1}}))",
          R"(Field(n,std::vector<std::vector<int>>{std::vector<int>{1,2},std::vector<int>{3}}))",
          "Field(e,std::vector<int>{})",
          "EndStruct()",
      }));
}

TEST(DumpStructTest, RawPointerField) {
  struct Pointer {
    int* p;
  };

  int i = 1;

  const std::vector<std::string> trace = Dump(Pointer{.p = &i});

  ASSERT_THAT(trace.size(), Eq(3u));
  EXPECT_THAT(trace[1].starts_with("Field(p,0x"), Eq(true)) << trace[1];
  EXPECT_THAT(trace[1], Not(Eq("Field(p,1)")));
}

TEST(DumpStructTest, ArrayMembers) {
  struct Arrays {
    int a[3];
    std::array<int, 3> b;
  };

  EXPECT_THAT(Dump(Arrays{.a = {1, 2, 3}, .b = {4, 5, 6}}),
              Eq(std::vector<std::string>{
                  "BeginStruct()",
                  "Field(a,int[3]{1,2,3})",
                  "BeginField(std::array<int, 3>,b)",
                  "BeginStruct()",
                  "Field(__elems_,int[3]{4,5,6})",
                  "EndStruct()",
                  "EndStruct()",
              }));
}

TEST(DumpStructTest, BalancedFrames) {
  struct A {
    std::string s;
  };

  struct B : A {
    int i;
  };

  struct C {
    int j;
  };

  struct D : B {
    int k;
    unsigned int l : 1;
    C c;
    B b;
    std::vector<int> v;
  };

  const std::vector<std::string> trace = Dump(D{B{A{.s = "x"}, /*i=*/1},
                                                /*k=*/2,
                                                /*l=*/1,
                                                /*c=*/{.j = 3},
                                                /*b=*/{A{.s = "y"}, /*i=*/4},
                                                /*v=*/{5, 6}});

  EXPECT_THAT(trace.front(), Eq("BeginStruct()"));
  EXPECT_THAT(trace.back(), Eq("EndStruct()"));
  EXPECT_THAT(std::count(trace.begin(), trace.end(), "BeginStruct()"),
              Eq(std::count(trace.begin(), trace.end(), "EndStruct()")));

  for (size_t i = 0; i < trace.size(); i++) {
    if (trace[i].starts_with("BeginField(")) {
      ASSERT_THAT(i + 1, Lt(trace.size())) << i;
      EXPECT_THAT(trace[i + 1], Eq("BeginStruct()"));
    }
  }
}

TEST(DumpStructTest, RepeatedDumps) {
  struct Base {
    int i;
  };

  struct Derived : Base {
    Base base;
    std::vector<int> v;
  };

  const Derived d{Base{.i = 1}, /*base=*/{.i = 2}, /*v=*/{3}};

  EXPECT_THAT(Dump(d), Eq(Dump(d)));
}

TEST(DumpStructTest, ReusedInstance) {
  struct Base {
    int i;
  };

  struct Derived : Base {
    int j;
  };

  const Derived d{Base{.i = 1}, /*j=*/2};
  const std::vector<std::string> once = Dump(d);

  MetaSink sink;
  DumpStruct<MetaSink> reused(&sink);
  __builtin_dump_struct(&d, reused);
  __builtin_dump_struct(&d, reused);

  std::vector<std::string> twice;
  twice.insert(twice.end(), once.begin(), once.end());
  twice.insert(twice.end(), once.begin(), once.end());

  EXPECT_THAT(sink.trace(), Eq(twice));
}

#endif  // __has_builtin(__builtin_dump_struct)

}  // namespace

}  // namespace pulse::reflect

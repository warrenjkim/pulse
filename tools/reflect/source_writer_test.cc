#include "tools/reflect/source_writer.h"

#include <string>
#include <string_view>
#include <utility>

#include "gtest/gtest.h"

namespace pulse::reflect::plugin {

namespace {

TEST(SourceWriterTest, Line) {
  SourceWriter w;
  w.Line("a", std::string("b"), std::string_view("c"));
  w.Line();
  EXPECT_EQ(*std::move(w), "abc\n\n");
}

TEST(SourceWriterTest, Indent) {
  SourceWriter w;
  w.Line("a");
  w.Indent();
  w.Line("b");
  w.Indent();
  w.Line("c");
  w.Line();
  EXPECT_EQ(*std::move(w), "a\n  b\n    c\n\n");
}

TEST(SourceWriterTest, Dedent) {
  SourceWriter w;
  w.Indent();
  w.Indent();
  w.Dedent();
  w.Line("a");
  w.Dedent();
  w.Line("b");
  EXPECT_EQ(*std::move(w), "  a\nb\n");
}

}  // namespace

}  // namespace pulse::reflect::plugin

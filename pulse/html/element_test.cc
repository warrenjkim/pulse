#include "pulse/html/element.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/html/attributes.h"
#include "pulse/html/tags.h"

namespace pulse::html {

namespace {

using ::testing::Eq;

TEST(RenderTest, TextOnly) {
  EXPECT_THAT(render(Make<P>("hello")), Eq("<p>hello</p>"));
}

TEST(RenderTest, EmptyElement) {
  EXPECT_THAT(render(Make<Div>()), Eq("<div></div>"));
}

TEST(RenderTest, SingleAttribute) {
  EXPECT_THAT(render(Make<Div>(Attributes{Class{"card"}})),
              Eq("<div class=\"card\"></div>"));
}

TEST(RenderTest, MultipleAttributes) {
  EXPECT_THAT(render(Make<Div>(Attributes{Class{"card"}, Id{"main"}})),
              Eq("<div class=\"card\" id=\"main\"></div>"));
}

TEST(RenderTest, NestedElements) {
  EXPECT_THAT(render(Make<Div>(Make<P>("hello"))),
              Eq("<div><p>hello</p></div>"));
}

TEST(RenderTest, AttributesAndChildren) {
  EXPECT_THAT(render(Make<Div>(Attributes{Class{"card"}}, Make<P>("hello"))),
              Eq("<div class=\"card\"><p>hello</p></div>"));
}

TEST(RenderTest, DeeplyNested) {
  EXPECT_THAT(
      render(Make<Div>(Attributes{Class{"card"}}, Make<H1>("heading 1"),
                       Make<P>("paragraph"))),
      Eq("<div class=\"card\"><h1>heading 1</h1><p>paragraph</p></div>"));
}

TEST(RenderTest, AllTags) {
  EXPECT_THAT(
      (render(Make<Html>(
          Make<Head>(Make<Title>("title")),
          Make<Body>(
              Make<H1>("heading 1"), Make<H2>("heading 2"),
              Make<H3>("heading 3"), Make<P>("paragraph"), Make<Span>("span"),
              Make<A>(Attributes{Href{"/link"}}, "anchor"),
              Make<Div>(Make<Table>(
                  Make<Thead>(Make<Tr>(Make<Th>("th"), Make<Th>("th"))),
                  Make<Tbody>(Make<Tr>(Make<Td>("td"), Make<Td>("td")),
                              Make<Tr>(Make<Td>("td"), Make<Td>("td"))))))))),
      Eq("<html>"
         "<head><title>title</title></head>"
         "<body>"
         "<h1>heading 1</h1>"
         "<h2>heading 2</h2>"
         "<h3>heading 3</h3>"
         "<p>paragraph</p>"
         "<span>span</span>"
         "<a href=\"/link\">anchor</a>"
         "<div>"
         "<table>"
         "<thead>"
         "<tr><th>th</th><th>th</th></tr>"
         "</thead>"
         "<tbody>"
         "<tr><td>td</td><td>td</td></tr>"
         "<tr><td>td</td><td>td</td></tr>"
         "</tbody>"
         "</table>"
         "</div>"
         "</body>"
         "</html>"));
}

}  // namespace

}  // namespace pulse::html

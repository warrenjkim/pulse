#include "pulse/html/element.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/html/attributes.h"
#include "pulse/html/tags.h"

namespace pulse::html {

namespace {

using ::testing::StrEq;

TEST(RenderTest, TextOnly) {
  EXPECT_THAT(Render(Make<P>("hello")), StrEq("<p>hello</p>"));
}

TEST(RenderTest, EmptyElement) {
  EXPECT_THAT(Render(Make<Div>()), StrEq("<div></div>"));
}

TEST(RenderTest, SingleAttribute) {
  EXPECT_THAT(Render(Make<Div>(Attributes{Class{"card"}})),
              StrEq("<div class=\"card\"></div>"));
}

TEST(RenderTest, MultipleAttributes) {
  EXPECT_THAT(Render(Make<Div>(Attributes{Class{"card"}, Id{"main"}})),
              StrEq("<div class=\"card\" id=\"main\"></div>"));
}

TEST(RenderTest, NestedElements) {
  EXPECT_THAT(Render(Make<Div>(Make<P>("hello"))),
              StrEq("<div><p>hello</p></div>"));
}

TEST(RenderTest, AttributesAndChildren) {
  EXPECT_THAT(Render(Make<Div>(Attributes{Class{"card"}}, Make<P>("hello"))),
              StrEq("<div class=\"card\"><p>hello</p></div>"));
}

TEST(RenderTest, DeeplyNested) {
  EXPECT_THAT(
      Render(Make<Div>(Attributes{Class{"card"}}, Make<H1>("heading 1"),
                       Make<P>("paragraph"))),
      StrEq("<div class=\"card\"><h1>heading 1</h1><p>paragraph</p></div>"));
}

TEST(RenderTest, AllTags) {
  EXPECT_THAT(
      (Render(Make<Html>(
          Make<Head>(Make<Title>("title")),
          Make<Body>(
              Make<H1>("heading 1"), Make<H2>("heading 2"),
              Make<H3>("heading 3"), Make<P>("paragraph"), Make<Span>("span"),
              Make<A>(Attributes{Href{"/link"}}, "anchor"),
              Make<Div>(Make<Table>(
                  Make<Thead>(Make<Tr>(Make<Th>("th"), Make<Th>("th"))),
                  Make<Tbody>(Make<Tr>(Make<Td>("td"), Make<Td>("td")),
                              Make<Tr>(Make<Td>("td"), Make<Td>("td"))))))))),
      StrEq("<html>"
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

TEST(RenderTest, FormTags) {
  EXPECT_THAT(Render(Make<Html>(
                  Make<Head>(Make<Title>("form")),
                  Make<Body>(Make<Form>(
                      Attributes{Action{"/submit"}, Method{"post"}},
                      Make<Label>(Attributes{For{"field"}}, "field"),
                      Make<Input>(Attributes{Type{"text"}, Name{"field"},
                                             Placeholder{"enter value"}}),
                      Make<Label>(Attributes{For{"choice"}}, "choice"),
                      Make<Select>(Attributes{Name{"choice"}},
                                   Make<Option>(Attributes{Value{"a"}}, "a"),
                                   Make<Option>(Attributes{Value{"b"}}, "b")),
                      Make<Button>(Attributes{Type{"submit"}}, "submit"))))),
              StrEq("<html>"
                    "<head><title>form</title></head>"
                    "<body>"
                    "<form action=\"/submit\" method=\"post\">"
                    "<label for=\"field\">field</label>"
                    "<input type=\"text\" name=\"field\" placeholder=\"enter "
                    "value\"></input>"
                    "<label for=\"choice\">choice</label>"
                    "<select name=\"choice\">"
                    "<option value=\"a\">a</option>"
                    "<option value=\"b\">b</option>"
                    "</select>"
                    "<button type=\"submit\">submit</button>"
                    "</form>"
                    "</body>"
                    "</html>"));
}

TEST(RenderTest, StyleTag) {
  EXPECT_THAT(Render(Make<Html>(Make<Head>(Make<Title>("styled"),
                                           Make<Style>("body { color: red; }")),
                                Make<Body>(Make<P>("hello")))),
              StrEq("<html>"
                    "<head>"
                    "<title>styled</title>"
                    "<style>body { color: red; }</style>"
                    "</head>"
                    "<body><p>hello</p></body>"
                    "</html>"));
}

}  // namespace

}  // namespace pulse::html

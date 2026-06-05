#pragma once

#include <concepts>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "pulse/core/overload.h"
#include "pulse/core/stringify.h"
#include "pulse/html/attributes.h"
#include "pulse/html/tags.h"
#include "pulse/html/text.h"
#include "pulse/strings/cat.h"

namespace pulse::html {

class Element;

template <typename T>
concept ChildType = std::convertible_to<T, Text> || std::same_as<T, Element>;

class Element {
 public:
  template <TagType Tag, AttributeType... Attrs, ChildType... Children>
  Element(Tag, Attributes<Attrs...> attributes, Children&&... children)
      : tag_(Tag::kTag) {
    std::apply(
        [this](auto&&... attrs) {
          (this->add_attribute<Tag>(
               std::forward<std::decay_t<decltype(attrs)>>(attrs)),
           ...);
        },
        attributes.attrs);

    (add_child(std::forward<Children>(children)), ...);
  }

 private:
  friend struct Stringify<Element>;

  using Node = std::variant<std::unique_ptr<Element>, Text>;

  template <TagType Tag, AttributeType Attr>
  void add_attribute(Attr&& attr) {
    static_assert(AttributeAllowed<std::decay_t<Attr>, Tag>,
                  "attribute not allowed on this tag");
    attributes_.push_back(static_cast<Attribute>(std::forward<Attr>(attr)));
  }

  template <ChildType Child>
  void add_child(Child&& child) {
    if constexpr (std::same_as<std::decay_t<Child>, Element>) {
      children_.push_back(
          std::make_unique<Element>(std::forward<Child>(child)));
    } else {
      children_.push_back(std::forward<Child>(child));
    }
  }

  std::string_view tag_;
  std::vector<Attribute> attributes_;
  std::vector<Node> children_;
};

template <TagType T, ChildType... Children>
Element Make(Children&&... children) {
  return Element(T{}, Attributes{}, std::forward<Children>(children)...);
}

template <TagType T, AttributeType... Attrs, ChildType... Children>
Element Make(Attributes<Attrs...> attributes, Children&&... children) {
  return Element(T{}, std::move(attributes),
                 std::forward<Children>(children)...);
}

std::string render(const Element& html);

}  // namespace pulse::html

template <>
struct pulse::Stringify<pulse::html::Element> {
  static std::string to_string(const pulse::html::Element& html) {
    std::string out;
    to_string(html, &out);
    return out;
  }

 private:
  static void to_string(const pulse::html::Element& html, std::string* out) {
    pulse::strings::append(out, "<", html.tag_);
    for (const pulse::html::Attribute& attribute : html.attributes_) {
      pulse::strings::append(out, " ", attribute.key, "=\"", attribute.value,
                             "\"");
    }

    pulse::strings::append(out, ">");
    for (const pulse::html::Element::Node& child : html.children_) {
      std::visit(
          pulse::Overload{
              [out](const pulse::html::Text& text) {
                pulse::strings::append(out, text);
              },
              [out](const std::unique_ptr<pulse::html::Element>& element) {
                pulse::Stringify<pulse::html::Element>::to_string(*element,
                                                                  out);
              }},
          child);
    }

    pulse::strings::append(out, "</", html.tag_, ">");
  }
};

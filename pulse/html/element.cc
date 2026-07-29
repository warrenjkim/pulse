#include "pulse/html/element.h"

#include <string>

#include "pulse/core/stringify.h"
#include "pulse/strings/cat.h"

namespace pulse::html {

std::string Render(const Element& element) {
  return Stringify<Element>::ToString(element);
}

std::string RenderDocument(const Element& html) {
  return pulse::strings::Cat("<!doctype html>", Render(html));
}

}  // namespace pulse::html

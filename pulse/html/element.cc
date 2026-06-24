#include "pulse/html/element.h"

#include <string>

#include "pulse/core/stringify.h"

namespace pulse::html {

std::string Render(const Element& element) {
  return Stringify<Element>::ToString(element);
}

}  // namespace pulse::html

#include "pulse/html/element.h"

#include <string>

#include "pulse/core/stringify.h"

namespace pulse::html {

std::string render(const Element& element) {
  return Stringify<Element>::to_string(element);
}

}  // namespace pulse::html

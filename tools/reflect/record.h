#pragma once

#include <string>
#include <vector>

namespace pulse::reflect::plugin {

// A class marked `[[pulse::reflect]]`, independent of Clang's AST.
struct Record {
  struct Field {
    std::string name;
  };

  // The class's name with its enclosing scopes, e.g. `geo::Point`.
  std::string qualified_name;
  std::vector<Field> fields;
};

}  // namespace pulse::reflect::plugin

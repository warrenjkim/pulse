#pragma once

#include <string>
#include <vector>

namespace pulse::reflect::plugin {

// A class marked `pulse_reflect`, independent of Clang's AST.
struct Record {
  // A namespace enclosing the class. An empty `name` is an anonymous namespace.
  struct Namespace {
    std::string name;
    bool is_inline{false};
  };

  struct Field {
    std::string name;
    // Non-public fields are read through `pulse::reflect::internal`.
    bool is_public{true};
  };

  // The class's name with its enclosing scopes, e.g. `geo::Point`.
  std::string qualified_name;
  std::vector<Namespace> namespaces;
  std::vector<Field> fields;
};

}  // namespace pulse::reflect::plugin

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
  };

  // The class's name with its enclosing scopes, e.g. `geo::Point`.
  std::string qualified_name;
  std::vector<Namespace> namespaces;
  std::vector<Field> fields;
};

}  // namespace pulse::reflect::plugin

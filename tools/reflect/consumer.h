#pragma once

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Decl.h"

namespace pulse::reflect::plugin {

// Selects each class marked `[[pulse::reflect]]` as its definition completes.
class ReflectConsumer : public clang::ASTConsumer {
 public:
  void HandleTagDeclDefinition(clang::TagDecl* tag) override;
};

}  // namespace pulse::reflect::plugin

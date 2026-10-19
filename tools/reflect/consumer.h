#pragma once

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/Diagnostic.h"

namespace pulse::reflect::plugin {

// Extracts each class marked `[[pulse::reflect]]` as its definition completes.
class ReflectConsumer : public clang::ASTConsumer {
 public:
  explicit ReflectConsumer(clang::DiagnosticsEngine* diags);

  void HandleTagDeclDefinition(clang::TagDecl* tag) override;

 private:
  clang::DiagnosticsEngine* diags_;
};

}  // namespace pulse::reflect::plugin

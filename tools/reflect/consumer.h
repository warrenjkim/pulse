#pragma once

#include <string>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Lex/Preprocessor.h"
#include "tools/reflect/record.h"

namespace pulse::reflect::plugin {

// Extracts each class marked `pulse_reflect` as its definition completes.
//
// A non-empty `emit_path` writes the reflection source of each class defined in
// the main file into `emit_path`. Otherwise, injects it after each definition.
class ReflectConsumer : public clang::ASTConsumer {
 public:
  explicit ReflectConsumer(clang::DiagnosticsEngine* diags,
                           clang::Preprocessor* preprocessor,
                           std::string emit_path);

  void HandleTagDeclDefinition(clang::TagDecl* tag) override;

  void HandleTranslationUnit(clang::ASTContext& context) override;

 private:
  clang::DiagnosticsEngine* diags_;
  clang::Preprocessor* preprocessor_;
  std::string emit_path_;
  std::vector<Record> records_;
};

}  // namespace pulse::reflect::plugin

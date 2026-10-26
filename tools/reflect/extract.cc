#include "tools/reflect/extract.h"

#include <optional>
#include <string>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Basic/TokenKinds.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/Token.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "tools/reflect/record.h"

namespace pulse::reflect::plugin {

namespace {

std::string QualifiedName(const clang::CXXRecordDecl& decl) {
  clang::PrintingPolicy policy(decl.getASTContext().getLangOpts());
  policy.SuppressUnwrittenScope = true;

  std::string name;
  llvm::raw_string_ostream out(name);
  decl.printQualifiedName(out, policy);
  return name;
}

void Error(clang::SourceLocation loc, llvm::StringRef message,
           clang::DiagnosticsEngine* diags) {
  diags->Report(loc, diags->getCustomDiagID(clang::DiagnosticsEngine::Error,
                                            "pulse::reflect: %0"))
      << message;
}

}  // namespace

std::optional<Record> Extract(const clang::CXXRecordDecl& decl,
                              clang::DiagnosticsEngine* diags) {
  const unsigned errors = diags->getNumErrors();

  if (decl.getDeclContext()->isFunctionOrMethod()) {
    Error(decl.getLocation(), "local classes cannot be reflected", diags);
  }

  // TODO(support nested classes)
  if (decl.getDeclContext()->isRecord()) {
    Error(decl.getLocation(), "nested classes are not supported yet", diags);
  }

  // TODO(support templated classes)
  if (decl.getDescribedClassTemplate() != nullptr ||
      llvm::isa<clang::ClassTemplateSpecializationDecl>(decl)) {
    Error(decl.getLocation(), "class templates are not supported yet", diags);
  }

  // TODO(support base classes)
  if (decl.getNumBases() > 0) {
    Error(decl.getLocation(), "base classes are not supported yet", diags);
  }

  const clang::ASTContext& context = decl.getASTContext();
  if (decl.getIdentifier() == nullptr) {
    Error(decl.getLocation(), "unnamed classes cannot be reflected", diags);
  } else if (const std::optional<clang::Token> next =
                 clang::Lexer::findNextToken(decl.getBraceRange().getEnd(),
                                             context.getSourceManager(),
                                             context.getLangOpts());
             next.has_value() && !next->is(clang::tok::semi)) {
    // TODO(support declarators after reflected classes)
    Error(next->getLocation(),
          "declare variables separately from a reflected class", diags);
  }

  Record record{.qualified_name = QualifiedName(decl)};
  for (const clang::FieldDecl* field : decl.fields()) {
    if (field->isAnonymousStructOrUnion()) {
      Error(field->getLocation(), "anonymous unions cannot be reflected",
            diags);
      continue;
    }

    // TODO(support bitfields)
    if (field->isBitField()) {
      Error(field->getLocation(), "bitfields are not supported yet", diags);
      continue;
    }

    // TODO(support private fields)
    if (field->getAccess() != clang::AS_public) {
      Error(field->getLocation(), "non-public fields are not supported yet",
            diags);
      continue;
    }

    record.fields.push_back({.name = field->getNameAsString()});
  }

  if (diags->getNumErrors() != errors) {
    return std::nullopt;
  }

  return record;
}

}  // namespace pulse::reflect::plugin

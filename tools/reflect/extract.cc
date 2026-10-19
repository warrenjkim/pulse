#include "tools/reflect/extract.h"

#include <optional>

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/Specifiers.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "tools/reflect/record.h"

namespace pulse::reflect::plugin {

namespace {

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

  if (decl.getIdentifier() == nullptr) {
    Error(decl.getLocation(), "unnamed classes cannot be reflected", diags);
  }

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

  Record record{.qualified_name = decl.getQualifiedNameAsString()};
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

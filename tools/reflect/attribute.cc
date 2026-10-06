#include "tools/reflect/attribute.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/ParsedAttrInfo.h"
#include "clang/Sema/ParsedAttr.h"
#include "clang/Sema/Sema.h"
#include "llvm/Support/Casting.h"

namespace pulse::reflect::plugin {

namespace {

constexpr clang::ParsedAttrInfo::Spelling kSpellings[] = {
    {.Syntax = clang::ParsedAttr::AS_CXX11,
     .NormalizedFullName = kAnnotation.data()}};

}  // namespace

PulseReflectAttrInfo::PulseReflectAttrInfo() { Spellings = kSpellings; }

bool PulseReflectAttrInfo::diagAppertainsToDecl(clang::Sema& sema,
                                                const clang::ParsedAttr& attr,
                                                const clang::Decl* decl) const {
  if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl);
      record != nullptr && !record->isUnion()) {
    return true;
  }

  sema.Diag(attr.getLoc(), sema.getDiagnostics().getCustomDiagID(
                               clang::DiagnosticsEngine::Error,
                               "%0 only applies to structs and classes"))
      << attr;
  return false;
}

PulseReflectAttrInfo::AttrHandling PulseReflectAttrInfo::handleDeclAttribute(
    clang::Sema& sema, clang::Decl* decl, const clang::ParsedAttr& attr) const {
  decl->addAttr(clang::AnnotateAttr::Create(sema.Context, kAnnotation, nullptr,
                                            0, attr.getRange()));
  return AttributeApplied;
}

bool MarkedWithPulseReflect(const clang::CXXRecordDecl& record) {
  for (const auto* attr : record.specific_attrs<clang::AnnotateAttr>()) {
    if (attr->getAnnotation() == kAnnotation) {
      return true;
    }
  }

  return false;
}

}  // namespace pulse::reflect::plugin

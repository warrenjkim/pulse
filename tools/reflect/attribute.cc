#include "tools/reflect/attribute.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DynamicRecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/Casting.h"

namespace pulse::reflect::plugin {

namespace {

class MisplacedMarkerVisitor : public clang::DynamicRecursiveASTVisitor {
 public:
  MisplacedMarkerVisitor(const clang::SourceManager& sources,
                         clang::DiagnosticsEngine* diags)
      : sources_(sources), diags_(diags) {}

  bool VisitDecl(clang::Decl* decl) override {
    if (!sources_.isInMainFile(decl->getLocation())) {
      return true;
    }

    if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl);
        record != nullptr && !record->isUnion()) {
      return true;
    }

    for (const auto* attr : decl->specific_attrs<clang::AnnotateAttr>()) {
      if (attr->getAnnotation() == kAnnotation) {
        diags_->Report(attr->getLocation(),
                       diags_->getCustomDiagID(
                           clang::DiagnosticsEngine::Error,
                           "pulse_reflect only applies to structs and classes"));
      }
    }

    return true;
  }

 private:
  const clang::SourceManager& sources_;
  clang::DiagnosticsEngine* diags_;
};

}  // namespace

bool MarkedWithPulseReflect(const clang::CXXRecordDecl& record) {
  for (const auto* attr : record.specific_attrs<clang::AnnotateAttr>()) {
    if (attr->getAnnotation() == kAnnotation) {
      return true;
    }
  }

  return false;
}

void ReportMisplacedMarkers(clang::ASTContext& context,
                            clang::DiagnosticsEngine* diags) {
  MisplacedMarkerVisitor(context.getSourceManager(), diags)
      .TraverseAST(context);
}

}  // namespace pulse::reflect::plugin

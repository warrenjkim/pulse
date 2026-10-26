#include "tools/reflect/consumer.h"

#include <optional>

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/Specifiers.h"
#include "llvm/Support/Casting.h"
#include "tools/reflect/attribute.h"
#include "tools/reflect/extract.h"
#include "tools/reflect/inject.h"
#include "tools/reflect/record.h"
#include "tools/reflect/render.h"

namespace pulse::reflect::plugin {

ReflectConsumer::ReflectConsumer(clang::DiagnosticsEngine* diags,
                                 clang::Preprocessor* preprocessor)
    : diags_(diags), preprocessor_(preprocessor) {}

void ReflectConsumer::HandleTagDeclDefinition(clang::TagDecl* tag) {
  const auto* decl = llvm::dyn_cast<clang::CXXRecordDecl>(tag);
  if (decl == nullptr || !MarkedWithPulseReflect(*decl)) {
    return;
  }

  // NOTE: Instantiations are handled through their template.
  if (clang::isTemplateInstantiation(decl->getTemplateSpecializationKind())) {
    return;
  }

  const std::optional<Record> record = Extract(*decl, diags_);
  if (!record.has_value()) {
    return;
  }

  if (!Inject(Render(*record), decl->getLocation(), preprocessor_)) {
    diags_->Report(decl->getLocation(),
                   diags_->getCustomDiagID(
                       clang::DiagnosticsEngine::Error,
                       "pulse::reflect: could not inject generated source"));
  }
}

}  // namespace pulse::reflect::plugin

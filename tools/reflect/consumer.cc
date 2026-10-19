#include "tools/reflect/consumer.h"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Basic/Specifiers.h"
#include "llvm/Support/Casting.h"
#include "tools/reflect/attribute.h"

namespace pulse::reflect::plugin {

void ReflectConsumer::HandleTagDeclDefinition(clang::TagDecl* tag) {
  const auto* decl = llvm::dyn_cast<clang::CXXRecordDecl>(tag);
  if (decl == nullptr || !MarkedWithPulseReflect(*decl)) {
    return;
  }

  // NOTE: Instantiations are handled through their template.
  if (clang::isTemplateInstantiation(decl->getTemplateSpecializationKind())) {
    return;
  }
}

}  // namespace pulse::reflect::plugin

#include "tools/reflect/attribute.h"

#include "clang/AST/Attr.h"
#include "clang/AST/DeclCXX.h"

namespace pulse::reflect::plugin {

bool MarkedWithPulseReflect(const clang::CXXRecordDecl& record) {
  for (const auto* attr : record.specific_attrs<clang::AnnotateAttr>()) {
    if (attr->getAnnotation() == kAnnotation) {
      return true;
    }
  }

  return false;
}

}  // namespace pulse::reflect::plugin

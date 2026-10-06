#pragma once

#include "clang/AST/DeclCXX.h"
#include "clang/Basic/ParsedAttrInfo.h"
#include "clang/Sema/ParsedAttr.h"
#include "llvm/ADT/StringRef.h"

namespace pulse::reflect::plugin {

inline constexpr llvm::StringLiteral kAnnotation = "pulse::reflect";

// Accepts `[[pulse::reflect]]` on structs and classes and marks them with
// `kAnnotation`.
//
// NOTE: Arguments are unsupported and ignored. So, `[[pulse::reflect(x)]]`
// currently behaves like `[[pulse::reflect]]`.
class PulseReflectAttrInfo : public clang::ParsedAttrInfo {
 public:
  PulseReflectAttrInfo();

  bool diagAppertainsToDecl(clang::Sema& sema, const clang::ParsedAttr& attr,
                            const clang::Decl* decl) const override;

  AttrHandling handleDeclAttribute(
      clang::Sema& sema, clang::Decl* decl,
      const clang::ParsedAttr& attr) const override;
};

// True if `record` is marked `[[pulse::reflect]]`.
bool MarkedWithPulseReflect(const clang::CXXRecordDecl& record);

}  // namespace pulse::reflect::plugin

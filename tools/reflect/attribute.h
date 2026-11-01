#pragma once

#include "clang/AST/DeclCXX.h"
#include "llvm/ADT/StringRef.h"

namespace pulse::reflect::plugin {

// The annotation `pulse_reflect` expands to.
inline constexpr llvm::StringLiteral kAnnotation = "pulse::reflect";

// True if `record` is marked `pulse_reflect`.
bool MarkedWithPulseReflect(const clang::CXXRecordDecl& record);

}  // namespace pulse::reflect::plugin

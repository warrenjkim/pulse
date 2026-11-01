#pragma once

#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Basic/Diagnostic.h"
#include "llvm/ADT/StringRef.h"

namespace pulse::reflect::plugin {

// The annotation `pulse_reflect` expands to.
inline constexpr llvm::StringLiteral kAnnotation = "pulse::reflect";

// True if `record` is marked `pulse_reflect`.
bool MarkedWithPulseReflect(const clang::CXXRecordDecl& record);

// Reports an error at each `pulse_reflect` in the main file that marks anything
// but a struct or class.
void ReportMisplacedMarkers(clang::ASTContext& context,
                            clang::DiagnosticsEngine* diags);

}  // namespace pulse::reflect::plugin

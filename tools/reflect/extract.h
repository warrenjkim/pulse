#pragma once

#include <optional>

#include "clang/AST/DeclCXX.h"
#include "clang/Basic/Diagnostic.h"
#include "tools/reflect/record.h"

namespace pulse::reflect::plugin {

// Describes `decl` as a `Record`, or reports an error at each unsupported part
// of it and returns `std::nullopt`.
std::optional<Record> Extract(const clang::CXXRecordDecl& decl,
                              clang::DiagnosticsEngine* diags);

}  // namespace pulse::reflect::plugin

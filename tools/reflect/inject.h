#pragma once

#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/StringRef.h"

namespace pulse::reflect::plugin {

// Makes Clang parse `source` as though it were written right after `loc`.
// Returns false if Clang couldn't enter the generated source.
[[nodiscard]] bool Inject(llvm::StringRef source, clang::SourceLocation loc,
                          clang::Preprocessor* preprocessor);

}  // namespace pulse::reflect::plugin

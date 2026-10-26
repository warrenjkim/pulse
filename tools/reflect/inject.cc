#include "tools/reflect/inject.h"

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"

namespace pulse::reflect::plugin {

bool Inject(llvm::StringRef source, clang::SourceLocation loc,
            clang::Preprocessor* preprocessor) {
  return !preprocessor->EnterSourceFile(
      preprocessor->getSourceManager().createFileID(
          llvm::MemoryBuffer::getMemBufferCopy(source, "<pulse::reflect>"),
          clang::SrcMgr::C_User, /*LoadedID=*/0, /*LoadedOffset=*/0,
          /*IncludeLoc=*/loc),
      /*Dir=*/nullptr, loc);
}

}  // namespace pulse::reflect::plugin

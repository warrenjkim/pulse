#include "tools/reflect/action.h"

#include <memory>
#include <string>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "llvm/ADT/StringRef.h"
#include "tools/reflect/consumer.h"

namespace pulse::reflect::plugin {

std::unique_ptr<clang::ASTConsumer> ReflectAction::CreateASTConsumer(
    clang::CompilerInstance& ci, llvm::StringRef) {
  return std::make_unique<ReflectConsumer>(
      &ci.getDiagnostics(), &ci.getPreprocessor(), emit_path_, include_);
}

bool ReflectAction::ParseArgs(const clang::CompilerInstance& ci,
                              const std::vector<std::string>& args) {
  for (llvm::StringRef arg : args) {
    if (arg.consume_front("emit=")) {
      emit_path_ = arg.str();
      continue;
    }

    if (arg.consume_front("include=")) {
      include_ = arg.str();
      continue;
    }

    clang::DiagnosticsEngine* diags = &ci.getDiagnostics();
    diags->Report(
        diags->getCustomDiagID(clang::DiagnosticsEngine::Error,
                               "pulse::reflect: unknown argument '%0'"))
        << arg;
    return false;
  }

  return true;
}

ReflectAction::ActionType ReflectAction::getActionType() {
  return ActionType::AddAfterMainAction;
}

}  // namespace pulse::reflect::plugin

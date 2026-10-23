#include "tools/reflect/action.h"

#include <memory>
#include <string>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/StringRef.h"
#include "tools/reflect/consumer.h"

namespace pulse::reflect::plugin {

std::unique_ptr<clang::ASTConsumer> ReflectAction::CreateASTConsumer(
    clang::CompilerInstance& ci, llvm::StringRef) {
  clang::Preprocessor& preprocessor = ci.getPreprocessor();
  preprocessor.setPredefines(preprocessor.getPredefines() +
                             "#define PULSE_REFLECT_PLUGIN 1\n");

  return std::make_unique<ReflectConsumer>(&ci.getDiagnostics(), &preprocessor);
}

bool ReflectAction::ParseArgs(const clang::CompilerInstance&,
                              const std::vector<std::string>&) {
  return true;
}

ReflectAction::ActionType ReflectAction::getActionType() {
  return ActionType::AddAfterMainAction;
}

}  // namespace pulse::reflect::plugin

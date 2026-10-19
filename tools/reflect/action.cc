#include "tools/reflect/action.h"

#include <memory>
#include <string>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "llvm/ADT/StringRef.h"

namespace pulse::reflect::plugin {

std::unique_ptr<clang::ASTConsumer> ReflectAction::CreateASTConsumer(
    clang::CompilerInstance&, llvm::StringRef) {
  return std::make_unique<clang::ASTConsumer>();
}

bool ReflectAction::ParseArgs(const clang::CompilerInstance&,
                              const std::vector<std::string>&) {
  return true;
}

ReflectAction::ActionType ReflectAction::getActionType() {
  return ActionType::AddAfterMainAction;
}

}  // namespace pulse::reflect::plugin

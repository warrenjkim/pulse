#pragma once

#include <memory>
#include <string>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "llvm/ADT/StringRef.h"

namespace pulse::reflect::plugin {

// Runs after Clang's main action on every file compiled with the plugin.
class ReflectAction : public clang::PluginASTAction {
 protected:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance& ci, llvm::StringRef file) override;

  bool ParseArgs(const clang::CompilerInstance& ci,
                 const std::vector<std::string>& args) override;

  ActionType getActionType() override;
};

}  // namespace pulse::reflect::plugin

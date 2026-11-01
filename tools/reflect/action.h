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
//
// Accepts `-fplugin-arg-pulse_reflect-emit=<path>` to write the generated
// source of the main file to `<path>` instead of injecting it, and
// `-fplugin-arg-pulse_reflect-include=<include>` to require the main file to
// include `<include>`, the generated source, after its reflected classes.
class ReflectAction : public clang::PluginASTAction {
 protected:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance& ci, llvm::StringRef file) override;

  bool ParseArgs(const clang::CompilerInstance& ci,
                 const std::vector<std::string>& args) override;

  ActionType getActionType() override;

 private:
  std::string emit_path_;
  std::string include_;
};

}  // namespace pulse::reflect::plugin

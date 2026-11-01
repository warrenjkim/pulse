#include "tools/reflect/consumer.h"

#include <optional>
#include <string>
#include <system_error>
#include <utility>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Specifiers.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "tools/reflect/attribute.h"
#include "tools/reflect/extract.h"
#include "tools/reflect/inject.h"
#include "tools/reflect/record.h"
#include "tools/reflect/render.h"

namespace pulse::reflect::plugin {

ReflectConsumer::ReflectConsumer(clang::DiagnosticsEngine* diags,
                                 clang::Preprocessor* preprocessor,
                                 std::string emit_path, std::string include)
    : diags_(diags),
      preprocessor_(preprocessor),
      emit_path_(std::move(emit_path)),
      include_(std::move(include)) {}

void ReflectConsumer::HandleTagDeclDefinition(clang::TagDecl* tag) {
  const auto* decl = llvm::dyn_cast<clang::CXXRecordDecl>(tag);
  if (decl == nullptr || !MarkedWithPulseReflect(*decl)) {
    return;
  }

  // NOTE: Instantiations are handled through their template.
  if (clang::isTemplateInstantiation(decl->getTemplateSpecializationKind())) {
    return;
  }

  // NOTE: When emitting, each header emits its own classes.
  if (!emit_path_.empty() &&
      !preprocessor_->getSourceManager().isInMainFile(decl->getLocation())) {
    return;
  }

  std::optional<Record> record = Extract(*decl, diags_);
  if (!record.has_value()) {
    return;
  }

  if (!emit_path_.empty()) {
    records_.push_back(*std::move(record));
    last_reflected_ = decl->getBraceRange().getEnd();
    return;
  }

  if (!Inject(RenderRecord(*record), decl->getLocation(), preprocessor_)) {
    diags_->Report(decl->getLocation(),
                   diags_->getCustomDiagID(
                       clang::DiagnosticsEngine::Error,
                       "pulse::reflect: could not inject generated source"));
  }
}

void ReflectConsumer::HandleTranslationUnit(clang::ASTContext&) {
  if (emit_path_.empty()) {
    return;
  }

  if (!include_.empty() && !records_.empty()) {
    CheckIncluded();
  }

  std::error_code error;
  llvm::raw_fd_ostream out(emit_path_, error);
  if (!error) {
    out << RenderHeader(records_);
    out.close();
    error = out.error();
  }

  if (error) {
    diags_->Report(
        diags_->getCustomDiagID(clang::DiagnosticsEngine::Error,
                                "pulse::reflect: could not write '%0': %1"))
        << emit_path_ << error.message();
  }
}

void ReflectConsumer::CheckIncluded() {
  const clang::SourceManager& sources = preprocessor_->getSourceManager();
  for (unsigned i = 0; i < sources.local_sloc_entry_size(); i++) {
    const clang::SrcMgr::SLocEntry& entry = sources.getLocalSLocEntry(i);
    if (!entry.isFile()) {
      continue;
    }

    const clang::SrcMgr::FileInfo& file = entry.getFile();
    const llvm::StringRef name = file.getName();
    if (!(name == include_ || name.ends_with("/" + include_)) ||
        !sources.isInMainFile(file.getIncludeLoc())) {
      continue;
    }

    if (sources.isBeforeInTranslationUnit(file.getIncludeLoc(),
                                          last_reflected_)) {
      diags_->Report(file.getIncludeLoc(),
                     diags_->getCustomDiagID(
                         clang::DiagnosticsEngine::Error,
                         "pulse::reflect: include \"%0\" after the last "
                         "reflected class"))
          << include_;
    }

    return;
  }

  diags_->Report(last_reflected_,
                 diags_->getCustomDiagID(
                     clang::DiagnosticsEngine::Error,
                     "pulse::reflect: this header does not include its "
                     "generated source; add `#include \"%0\"` after its "
                     "reflected classes"))
      << include_;
}

}  // namespace pulse::reflect::plugin

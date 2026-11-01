#include "tools/reflect/render.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "tools/reflect/record.h"
#include "tools/reflect/source_writer.h"

namespace pulse::reflect::plugin {

namespace {

constexpr std::string_view kDescriptor = "PulseField";

// Returns `, PulseField0, PulseField1, ...` for `count` descriptors.
std::string FieldDescriptors(size_t count) {
  std::string out;
  out.reserve(count * (2 + kDescriptor.size() + 3));
  for (size_t i = 0; i < count; i++) {
    out += ", ";
    out += kDescriptor;
    out += std::to_string(i);
  }

  return out;
}

void WriteRecord(const Record& record, SourceWriter* w) {
  const std::string& qualified_name = record.qualified_name;

  w->Line(
      "[[maybe_unused]] "
      "consteval auto PulseReflect(const ::",
      qualified_name, "*) {");
  w->Indent();

  for (size_t i = 0; i < record.fields.size(); i++) {
    const Record::Field& field = record.fields[i];

    w->Line("struct ", kDescriptor, std::to_string(i), " {");
    w->Indent();
    w->Line("using Type = decltype(::", qualified_name, "::", field.name, ");");
    w->Line("static consteval const char* Name() { return \"", field.name,
            "\"; }");
    w->Line("static constexpr const Type& Value(const ::", qualified_name,
            "& v) { return v.", field.name, "; }");
    w->Dedent();
    w->Line("};");
  }

  w->Line("return ::pulse::reflect::Mirror<::", qualified_name,
          FieldDescriptors(record.fields.size()), ">{};");
  w->Dedent();
  w->Line("}");
}

}  // namespace

std::string RenderRecord(const Record& record) {
  SourceWriter w;
  WriteRecord(record, &w);
  return *std::move(w);
}

std::string RenderHeader(const std::vector<Record>& records) {
  SourceWriter w;
  w.Line("#pragma once");
  if (records.empty()) {
    return *std::move(w);
  }

  w.Line();
  w.Line(R"src(#include "pulse/reflect/mirror.h")src");
  for (const Record& record : records) {
    w.Line();
    for (const Record::Namespace& ns : record.namespaces) {
      w.Line(ns.is_inline ? "inline " : "", "namespace",
             ns.name.empty() ? "" : " ", ns.name, " {");
    }

    WriteRecord(record, &w);
    for (auto ns = record.namespaces.rbegin(); ns != record.namespaces.rend();
         ns++) {
      w.Line("}  // namespace", ns->name.empty() ? "" : " ", ns->name);
    }
  }

  return *std::move(w);
}

}  // namespace pulse::reflect::plugin

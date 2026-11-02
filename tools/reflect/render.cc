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

// Returns `::pulse::reflect::internal::PrivateField<::ns::A, 1>` for
// `name` = "PrivateField".
std::string PrivateField(std::string_view name, std::string_view qualified_name,
                         size_t i) {
  std::string out("::pulse::reflect::internal::");
  out += name;
  out += "<::";
  out += qualified_name;
  out += ", ";
  out += std::to_string(i);
  out += ">";
  return out;
}

// Defines `GetMemberPointer` for each non-public field of `record`, at global
// scope. See `pulse::reflect::internal::MemberPointer`.
void WriteMemberPointers(const Record& record, SourceWriter* w) {
  bool opened = false;
  for (size_t i = 0; i < record.fields.size(); i++) {
    const Record::Field& field = record.fields[i];
    if (field.is_public) {
      continue;
    }

    if (!opened) {
      w->Line("namespace pulse::reflect::internal {");
      opened = true;
    }

    const std::string tag =
        PrivateField("PrivateField", record.qualified_name, i);
    w->Line("constexpr auto GetMemberPointer(", tag, ");");
    w->Line("template struct MemberPointer<", tag,
            ", &::", record.qualified_name, "::", field.name, ">;");
  }

  if (opened) {
    w->Line("}  // namespace pulse::reflect::internal");
    w->Line();
  }
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

    const std::string index = std::to_string(i);
    if (field.is_public) {
      w->Line("struct ", kDescriptor, index, " {");
      w->Indent();
      w->Line("using Type = decltype(::", qualified_name, "::", field.name,
              ");");
    } else {
      w->Line("struct ", kDescriptor, index, " : ",
              PrivateField("PrivateFieldDescriptor", qualified_name, i), " {");
      w->Indent();
    }

    w->Line("static consteval const char* Name() { return \"", field.name,
            "\"; }");
    if (field.is_public) {
      w->Line("static constexpr const Type& Value(const ::", qualified_name,
              "& v) { return v.", field.name, "; }");
    }

    w->Dedent();
    w->Line("};");
  }

  w->Line("return ::pulse::reflect::Mirror<::", qualified_name,
          FieldDescriptors(record.fields.size()), ">{};");
  w->Dedent();
  w->Line("}");
}

}  // namespace

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
    WriteMemberPointers(record, &w);
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

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

#include "pulse/core/stringify.h"
#include "pulse/reflect/data_sink.h"

namespace pulse::reflect {

// A `DataSink` that renders a struct traversal as `Type{.field=value,...}`.
//
// NOTE: Nested struct labels come from the traversal's own type spelling, which
// need not match `pulse::internal::TypeLabel`. The outermost label is not
// emitted here -- the caller knows the static type and prepends it.
class StringifySink {
 public:
  void BeginStruct() {
    Separate();
    out_ += "{";
    first_ = true;
  }

  void EndStruct() {
    out_ += "}";
    first_ = false;
  }

  void BeginField(std::string_view type, std::string_view name) {
    Separate();
    out_ += ".";
    out_ += name;
    out_ += "=";
    out_ += type;
    first_ = true;
  }

  template <Stringifiable T>
  void Field(std::string_view name, const T& value) {
    Separate();
    out_ += ".";
    out_ += name;
    out_ += "=";
    out_ += pulse::ToString(value);
  }

  const std::string& out() const& { return out_; }

  std::string out() && { return std::move(out_); }

 private:
  void Separate() {
    if (!first_) {
      out_ += ",";
    }

    first_ = false;
  }

  std::string out_;
  bool first_{true};
};
static_assert(DataSink<StringifySink>);

}  // namespace pulse::reflect

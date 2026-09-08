#pragma once

#include <string_view>

#include "pulse/core/stringifiable_type.h"

namespace pulse::reflect {

// A sink for reflecting an instance's data members. It defines what happens at
// each step of a struct traversal. The sink is not responsible for how the
// struct is traversed.
template <typename S>
concept DataSink =
    requires(S& sink, std::string_view type, std::string_view name,
             const pulse::internal::StringifiableType& v) {
      // Enter a struct scope.
      //
      // NOTE: Should be called on both the outermost struct being reflected as
      // well as any struct-valued field.
      sink.BeginStruct();

      // Exit a struct scope.
      //
      // NOTE: Should be called on both the outermost struct being reflected as
      // well as any struct-valued field.
      sink.EndStruct();

      // Emit the type and name of the field.
      //
      // NOTE: Should be called immediately before `BeginStruct` for that
      // field's value.
      sink.BeginField(type, name);

      // Emit the name and value of a leaf field.
      //
      // NOTE: Should be called for any field value that is not a struct. Should
      // never be paired with `BeginField`.
      //
      // NOTE: `v` is a stand-in for any `Stringifiable` type. This forces
      // `Field` to be generic over `Stringifiable` rather than a fixed set of
      // types.
      sink.Field(name, v);
    };

}  // namespace pulse::reflect

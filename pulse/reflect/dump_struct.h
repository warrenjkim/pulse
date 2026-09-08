#pragma once

#include <string_view>
#include <type_traits>
#include <vector>

#include "pulse/reflect/data_sink.h"

namespace pulse::reflect {

#if __has_builtin(__builtin_dump_struct)
inline constexpr bool kHasBuiltinDumpStruct = true;
#else
inline constexpr bool kHasBuiltinDumpStruct = false;
#endif

// Translates Clang's `__builtin_dump_struct` into `DataSink`.
//
// NOTE: Inheritance hierarchies are flattened. Inherited fields are emitted as
// fields of the derived struct, and the base struct's boundaries are omitted.
template <DataSink S>
  requires(kHasBuiltinDumpStruct)
class DumpStruct {
 public:
  explicit DumpStruct(S* sink) : sink_(sink) {}

  // Translates `__builtin_dump_struct` to `DataSink`.
  template <typename... Args>
  void operator()(const char* fmt, Args... args);

 private:
  // Begins a Clang traversal frame.
  //
  // NOTE: Base-class frames are tracked but omitted from the `DataSink` so
  // their fields are flattened into the enclosing struct.
  void BeginFrame();

  // Ends a Clang traversal frame.
  //
  // NOTE: Closes the corresponding struct in the `DataSink` unless the frame
  // represents a base class.
  void EndFrame();

  // Decodes a Clang field callback. The shape of each callback identifies what
  // is being described:
  //
  //   ()                          open/close a frame
  //   (type)                      the frame's type spelling
  //   (indent)                    close a nested frame
  //   (indent, type)              a base-class frame
  //   (indent, type, name)        a field containing a nested frame
  //   (indent, type, name, v)     a leaf field
  //   (indent, type, name, w, v)  a bitfield and its width
  template <typename... Rest>
  void DecodeField(std::string_view fmt, const char* indent, const char* type,
                   const char* name, Rest... rest);

  // Forwards a field value to the `DataSink`.
  template <typename V>
  void DecodeField(std::string_view fmt, const char* name, V value);

  // Forwards a bitfield value to the `DataSink`.
  //
  // NOTE: The width is ignored because there is no equivalent representation in
  // `DataSink`.
  template <typename W, typename V>
  void DecodeField(std::string_view fmt, const char* name, W width, V value);

  // Forwards the field's value to the `DataSink`.
  //
  // NOTE: Fields Clang cannot format itself are emitted as `*%p` plus a pointer
  // to the field. The pointer retains the field's static type, allowing the
  // value to be dereferenced and forwarded to the `DataSink`. Every other
  // pointer is forwarded as its address.
  template <typename V>
  void ForwardValue(std::string_view fmt, const char* name, V value);

  S* sink_;
  bool next_frame_is_base_{false};
  std::vector<bool> open_frame_is_base_;
};

template <DataSink S>
  requires(kHasBuiltinDumpStruct)
template <typename... Args>
void DumpStruct<S>::operator()(const char* fmt, Args... args) {
  const std::string_view f(fmt);

  if constexpr (sizeof...(Args) == 0) {
    if (f.find('{') != std::string_view::npos) {
      BeginFrame();
    } else if (f.find('}') != std::string_view::npos) {
      EndFrame();
    }
  } else if constexpr (sizeof...(Args) == 1) {
    if (f.find('}') != std::string_view::npos) {
      EndFrame();
    }
  } else if constexpr (sizeof...(Args) == 2) {
    next_frame_is_base_ = true;
  } else {
    DecodeField(f, args...);
  }
}

template <DataSink S>
  requires(kHasBuiltinDumpStruct)
void DumpStruct<S>::BeginFrame() {
  const bool is_base = next_frame_is_base_;
  next_frame_is_base_ = false;

  open_frame_is_base_.push_back(is_base);

  if (!is_base) {
    sink_->BeginStruct();
  }
}

template <DataSink S>
  requires(kHasBuiltinDumpStruct)
void DumpStruct<S>::EndFrame() {
  const bool is_base = open_frame_is_base_.back();
  open_frame_is_base_.pop_back();

  if (!is_base) {
    sink_->EndStruct();
  }
}

template <DataSink S>
  requires(kHasBuiltinDumpStruct)
template <typename... Rest>
void DumpStruct<S>::DecodeField(std::string_view fmt, const char* /*indent*/,
                                const char* type, const char* name,
                                Rest... rest) {
  if constexpr (sizeof...(Rest) == 0) {
    sink_->BeginField(type, name);
  } else {
    DecodeField(fmt, name, rest...);
  }
}

template <DataSink S>
  requires(kHasBuiltinDumpStruct)
template <typename V>
void DumpStruct<S>::DecodeField(std::string_view fmt, const char* name,
                                V value) {
  ForwardValue(fmt, name, value);
}

template <DataSink S>
  requires(kHasBuiltinDumpStruct)
template <typename W, typename V>
void DumpStruct<S>::DecodeField(std::string_view fmt, const char* name,
                                W /*width*/, V value) {
  ForwardValue(fmt, name, value);
}

template <DataSink S>
  requires(kHasBuiltinDumpStruct)
template <typename V>
void DumpStruct<S>::ForwardValue(std::string_view fmt, const char* name,
                                 V value) {
  if constexpr (!std::is_pointer_v<V>) {
    sink_->Field(name, value);
  } else if constexpr (std::is_function_v<std::remove_pointer_t<V>>) {
    sink_->Field(name, reinterpret_cast<const void*>(value));
  } else {
    if constexpr (requires { sink_->Field(name, *value); }) {
      if (fmt.ends_with("*%p\n")) {
        sink_->Field(name, *value);
        return;
      }
    }

    sink_->Field(name, const_cast<const void*>(
                           static_cast<const volatile void*>(value)));
  }
}

}  // namespace pulse::reflect

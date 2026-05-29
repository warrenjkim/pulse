#pragma once

#include <tuple>
#include <type_traits>

namespace pulse {

// A map that stores and retrieves values by type. All storable types must be
// declared upfront as template parameters.
//
// Usage:
//
//   TypeMap<int, float, std::string> map;
//   map.set(42);
//   map.set(3.14f);
//   map.set<std::string>("hello");
//
//   const int& i = map.get<int>();  // const T& get()
//   float f = map.get<float>();  // const T& get() calls copy constructor
//   std::string& s = map.get<std::string>();  // T& get()
//
// Note: get() returns a default constructed value if set() has not been
// called for that type.
template <typename... Types>
class TypeMap {
 public:
  template <typename T>
    requires(std::is_same_v<T, Types> || ...)
  void set(T value) {
    std::get<T>(map_) = std::move(value);
  }

  template <typename T>
    requires(std::is_same_v<T, Types> || ...)
  const T& get() const {
    return std::get<T>(map_);
  }

  template <typename T>
    requires(std::is_same_v<T, Types> || ...)
  T& get() {
    return std::get<T>(map_);
  }

 private:
  std::tuple<Types...> map_;
};

}  // namespace pulse

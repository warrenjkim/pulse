#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "pulse/core/stringify.h"

template <typename T>
struct pulse::Stringify<std::vector<T>> {
  static std::string ToString(const std::vector<T>& vector) {
    std::string out = "std::vector{";
    for (const auto& value : vector) {
      out += pulse::Stringify<T>::ToString(value) + ",";
    }

    if (out.back() == ',') {
      out.pop_back();
    }

    return out + "}";
  }
};

template <typename K, typename V>
struct pulse::Stringify<std::unordered_map<K, V>> {
  static std::string ToString(const std::unordered_map<K, V>& map) {
    std::string out = "std::unordered_map{";
    for (const auto& [key, value] : map) {
      out += "{" + pulse::Stringify<K>::ToString(key) + "," +
             pulse::Stringify<V>::ToString(value) + "},";
    }

    if (!map.empty()) {
      out.pop_back();
    }

    return out + "}";
  }
};

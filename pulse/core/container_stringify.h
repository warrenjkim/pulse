#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "core/stringify.h"

template <typename T>
struct pulse::Stringify<std::vector<T>> {
  static std::string to_string(const std::vector<T>& vector) {
    std::string out = "std::vector{";
    for (const auto& value : vector) {
      out += pulse::Stringify<T>::to_string(value) + ",";
    }

    if (out.back() == ',') {
      out.pop_back();
    }

    return out + "}";
  }
};

template <typename K, typename V>
struct pulse::Stringify<std::unordered_map<K, V>> {
  static std::string to_string(const std::unordered_map<K, V>& map) {
    std::string out = "std::unordered_map{";
    for (const auto& [key, value] : map) {
      out += "{" + pulse::Stringify<K>::to_string(key) + "," +
             pulse::Stringify<V>::to_string(value) + "},";
    }

    if (!map.empty()) {
      out.pop_back();
    }

    return out + "}";
  }
};

#pragma once

#include <map>
#include <memory>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace pulse {

template <typename K, typename V>
class Trie;

template <typename K, typename V>
std::string to_string(const Trie<K, V>& trie);

template <typename K, typename V>
std::ostream& operator<<(std::ostream& os, const Trie<K, V>& trie);

template <typename K, typename V>
class Trie {
 public:
  using E = typename K::value_type;

  explicit Trie();

  Trie(const Trie& other);
  Trie& operator=(const Trie& other);

  template <typename KeyRange>
  void insert(const KeyRange& key, V value);

  template <typename KeyRange>
  bool prefix(const KeyRange& key) const;

  template <typename KeyRange>
  bool match(const KeyRange& key) const;

  template <typename KeyRange>
  V& operator[](const KeyRange& key);

  template <typename KeyRange>
  const V* get(const KeyRange& key) const;

  template <typename KeyRange>
  bool erase(const KeyRange& key);

 private:
  struct Node {
    std::optional<V> value;
    std::map<E, std::unique_ptr<Node>> children;
  };

  template <typename KeyRange>
  static auto make_span(const KeyRange& key);

  static std::unique_ptr<Node> clone(const Node* node);

  friend std::string to_string<>(const Trie<K, V>& trie);

  std::unique_ptr<Node> root_;
};

template <typename K, typename V>
Trie<K, V>::Trie() : root_(std::make_unique<Node>()) {}

template <typename K, typename V>
Trie<K, V>::Trie(const Trie& other) : root_(clone(other)) {}

template <typename K, typename V>
Trie<K, V>& Trie<K, V>::operator=(const Trie& other) {
  if (this != &other) {
    root_ = clone(other.root_.get());
  }
  return *this;
}

template <typename K, typename V>
template <typename KeyRange>
void Trie<K, V>::insert(const KeyRange& key, V value) {
  Node* curr = root_.get();
  for (const E& c : make_span(key)) {
    auto& child = curr->children[c];
    if (!child) {
      child = std::make_unique<Node>();
    }

    curr = child.get();
  }

  curr->value = std::move(value);
}

template <typename K, typename V>
template <typename KeyRange>
bool Trie<K, V>::prefix(const KeyRange& key) const {
  const Node* curr = root_.get();
  for (const E& c : make_span(key)) {
    if (auto it = curr->children.find(c); it != curr->children.end()) {
      curr = it->second.get();
    } else {
      return false;
    }
  }

  return true;
}

template <typename K, typename V>
template <typename KeyRange>
bool Trie<K, V>::match(const KeyRange& key) const {
  const Node* curr = root_.get();
  for (const E& c : make_span(key)) {
    if (auto it = curr->children.find(c); it != curr->children.end()) {
      curr = it->second.get();
    } else {
      return false;
    }
  }

  return curr->value.has_value();
}

template <typename K, typename V>
template <typename KeyRange>
V& Trie<K, V>::operator[](const KeyRange& key) {
  Node* curr = root_.get();
  for (const E& c : make_span(key)) {
    auto& child = curr->children[c];
    if (!child) {
      child = std::make_unique<Node>();
    }

    curr = child.get();
  }

  return *curr->value;
}

template <typename K, typename V>
template <typename KeyRange>
const V* Trie<K, V>::get(const KeyRange& key) const {
  const Node* curr = root_.get();
  for (const E& c : make_span(key)) {
    if (auto it = curr->children.find(c); it != curr->children.end()) {
      curr = it->second.get();
    } else {
      return nullptr;
    }
  }

  return curr->value.has_value() ? &(*curr->value) : nullptr;
}

template <typename K, typename V>
template <typename KeyRange>
bool Trie<K, V>::erase(const KeyRange& key) {
  auto remove = [](auto& self, Node* node, auto curr, auto end) -> bool {
    if (curr == end) {
      if (!node->value) {
        return false;
      }

      node->value.reset();
      return true;
    }

    auto it = node->children.find(*curr);
    if (it == node->children.end()) {
      return false;
    }

    bool removed = self(self, it->second.get(), ++curr, end);
    if (removed && !it->second->value && it->second->children.empty()) {
      node->children.erase(it);
    }

    return removed;
  };

  auto span = make_span(key);
  return remove(remove, root_.get(), span.begin(), span.end());
}

template <typename K, typename V>
template <typename KeyRange>
auto Trie<K, V>::make_span(const KeyRange& key) {
  if constexpr (std::is_convertible_v<KeyRange, std::basic_string_view<E>>) {
    return std::basic_string_view<E>(key);
  } else {
    return std::span(key);
  }
}

template <typename K, typename V>
std::unique_ptr<typename Trie<K, V>::Node> Trie<K, V>::clone(const Node* node) {
  if (!node) {
    return nullptr;
  }

  auto copy = std::make_unique<Node>();
  copy->value = node->value;
  for (const auto& [c, child] : node->children) {
    copy->children[c] = CloneNode(child.get());
  }

  return copy;
}

template <typename K, typename V>
std::string to_string(const Trie<K, V>& trie) {
  if (trie.root_->children.empty()) {
    return "";
  }

  std::vector<std::string> lines;

  auto partial = [&lines](auto& self, const typename Trie<K, V>::Node* node,
                          const typename Trie<K, V>::E& key,
                          std::vector<size_t> relatives) -> void {
    std::string token;
    {
      std::ostringstream ss;
      ss << key;

      token = std::move(ss).str();
    }

    if (node->value.has_value()) {
      lines.back().append("(").append(std::move(token)).append(")");
    } else {
      lines.back().append(std::move(token));
    }

    if (node->children.empty()) {
      return;
    }

    size_t anchor = lines.back().size();
    if (node->children.size() > 1) {
      relatives.push_back(anchor);
    }

    bool first = true;
    size_t children = 0;
    for (const auto& [c, child] : node->children) {
      children++;
      if (first) {
        first = false;
        lines.back().append("-");
        self(self, child.get(), c, relatives);
      } else {
        lines.emplace_back();
        if (children == node->children.size()) {
          relatives.pop_back();
        }

        std::string prefix(anchor, ' ');
        for (size_t bar : relatives) {
          if (bar < anchor) {
            prefix[bar] = '|';
          }
        }

        lines.back().append(prefix).append("`-");
        self(self, child.get(), c, relatives);
      }
    }
  };

  for (const auto& [key, node] : trie.root_->children) {
    lines.emplace_back();
    partial(partial, node.get(), key, {});
  }

  std::string out;
  size_t size =
      static_cast<size_t>(std::max(static_cast<int>(lines.size()) - 1, 0));
  for (const std::string& line : lines) {
    size += line.size();
  }

  out.reserve(size);
  for (std::string& line : lines) {
    out.append(std::move(line)).append("\n");
  }

  if (!out.empty()) {
    out.pop_back();
  }

  return out;
}

template <typename K, typename V>
std::ostream& operator<<(std::ostream& os, const Trie<K, V>& trie) {
  os << to_string(trie);
  return os;
}

}  // namespace pulse

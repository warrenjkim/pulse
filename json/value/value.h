#pragma once

#include <cstddef>  // nullptr_t, size_t
#include <cstdint>  // int32_t
#include <map>
#include <string>
#include <vector>

#include "pulse/json/utils/exception.h"

namespace pulse {
namespace json {

class Value;
using array_t = std::vector<Value>;
using object_t = std::map<std::string, Value>;

class Value {
 public:
  Value() noexcept : type_(Type::JSON_NULL) {}

  ~Value() noexcept { destroy(); }

  Value(const Value& other) {
    switch (other.type_) {
      case Type::ARRAY:
        ::new ((void*)(&a_)) array_t(other.a_);
        break;
      case Type::BOOLEAN:
        b_ = other.b_;
        break;
      case Type::JSON_NULL:
        break;
      case Type::INTEGRAL:
      case Type::DOUBLE:
        n_ = other.n_;
        break;
      case Type::OBJECT:
        ::new ((void*)(&o_)) object_t(other.o_);
        break;
      case Type::STRING:
        ::new ((void*)(&s_)) std::string(other.s_);
        break;
    }

    type_ = other.type_;
  }

  Value(Value&& other) noexcept {
    switch (other.type_) {
      case Type::ARRAY:
        ::new ((void*)(&a_)) array_t(std::move(other.a_));
        break;
      case Type::BOOLEAN:
        b_ = other.b_;
        break;
      case Type::JSON_NULL:
        break;
      case Type::INTEGRAL:
      case Type::DOUBLE:
        n_ = other.n_;
        break;
      case Type::OBJECT:
        ::new ((void*)(&o_)) object_t(std::move(other.o_));
        break;
      case Type::STRING:
        ::new ((void*)(&s_)) std::string(std::move(other.s_));
        break;
    }

    type_ = other.type_;
    other.destroy();
  }

  Value(nullptr_t) noexcept : type_(Type::JSON_NULL) {}

  Value(bool b) noexcept : b_(b), type_(Type::BOOLEAN) {}

  Value(int32_t n) noexcept : n_(n), type_(Type::INTEGRAL) {}

  Value(double n) noexcept : n_(n), type_(Type::DOUBLE) {}

  Value(array_t a) {
    ::new ((void*)(&a_)) array_t(std::move(a));
    type_ = Type::ARRAY;
  }

  Value(object_t o) {
    ::new ((void*)(&o_)) object_t(std::move(o));
    type_ = Type::OBJECT;
  }

  Value(const char* s) {
    ::new ((void*)(&s_)) std::string(s);
    type_ = Type::STRING;
  }

  Value(std::string s) noexcept {
    ::new ((void*)(&s_)) std::string(std::move(s));
    type_ = Type::STRING;
  }

  Value& operator=(const Value& other) {
    if (this != &other) {
      destroy();
      switch (other.type_) {
        case Type::ARRAY:
          ::new ((void*)(&a_)) array_t(other.a_);
          break;
        case Type::BOOLEAN:
          b_ = other.b_;
          break;
        case Type::JSON_NULL:
          break;
        case Type::INTEGRAL:
        case Type::DOUBLE:
          n_ = other.n_;
          break;
        case Type::OBJECT:
          ::new ((void*)(&o_)) object_t(other.o_);
          break;
        case Type::STRING:
          ::new ((void*)(&s_)) std::string(other.s_);
          break;
      }

      type_ = other.type_;
    }

    return *this;
  }

  Value& operator=(Value&& other) noexcept {
    if (this != &other) {
      destroy();
      switch (other.type_) {
        case Type::ARRAY:
          ::new ((void*)(&a_)) array_t(std::move(other.a_));
          break;
        case Type::BOOLEAN:
          b_ = other.b_;
          break;
        case Type::JSON_NULL:
          break;
        case Type::INTEGRAL:
        case Type::DOUBLE:
          n_ = other.n_;
          break;
        case Type::OBJECT:
          ::new ((void*)(&o_)) object_t(std::move(other.o_));
          break;
        case Type::STRING:
          ::new ((void*)(&s_)) std::string(std::move(other.s_));
          break;
      }

      type_ = other.type_;
      other.destroy();
    }

    return *this;
  }

  operator const array_t&() const {
    assert_type(Type::ARRAY);
    return a_;
  }

  operator array_t&() {
    assert_type(Type::ARRAY);
    return a_;
  }

  operator bool() const {
    assert_type(Type::BOOLEAN);
    return b_;
  }

  operator double() const {
    assert_type(Type::DOUBLE);
    return n_;
  }

  operator float() const {
    assert_type(Type::DOUBLE);
    return float(n_);
  }

  operator int32_t() const {
    assert_type(Type::INTEGRAL);
    return int32_t(n_);
  }

  operator const object_t&() const {
    assert_type(Type::OBJECT);
    return o_;
  }

  operator object_t&() {
    assert_type(Type::OBJECT);
    return o_;
  }

  operator std::string&() {
    assert_type(Type::STRING);
    return s_;
  }

  operator const std::string&() const {
    assert_type(Type::STRING);
    return s_;
  }

  operator const char*() const {
    assert_type(Type::STRING);
    return s_.c_str();
  }

  bool operator==(const Value& other) const {
    bool is_number =
        ((type_ == Type::INTEGRAL && other.type_ == Type::DOUBLE) ||
         (type_ == Type::DOUBLE && other.type_ == Type::INTEGRAL));
    if (type_ != other.type_ && !is_number) {
      return false;
    }

    switch (type_) {
      case Type::ARRAY:
        return a_ == other.a_;
      case Type::BOOLEAN:
        return b_ == other.b_;
      case Type::JSON_NULL:
        return true;
      case Type::INTEGRAL:
      case Type::DOUBLE:
        return n_ == other.n_;
      case Type::OBJECT:
        return o_ == other.o_;
      case Type::STRING:
        return s_ == other.s_;
    }

    __builtin_unreachable();
  }

  bool operator==(nullptr_t) const noexcept { return type_ == Type::JSON_NULL; }

  bool operator==(bool b) const noexcept {
    return type_ == Type::BOOLEAN && b == b_;
  }

  bool operator==(double n) const noexcept {
    return type_ == Type::DOUBLE && n == n_;
  }

  bool operator==(int32_t n) const noexcept {
    return type_ == Type::INTEGRAL && n == int32_t(n_);
  }

  bool operator==(const std::string& s) const noexcept {
    return type_ == Type::STRING && s == s_;
  }

  bool operator==(const char* s) const noexcept {
    return type_ == Type::STRING && s == s_;
  }

  // containers
  size_t size() const {
    switch (type_) {
      case Type::ARRAY:
        return a_.size();
      case Type::OBJECT:
        return o_.size();
      default:
        throw BadAccessException(
            "expected container type (array, object), got " + type(type_));
    }

    __builtin_unreachable();
  }

  bool empty() const {
    switch (type_) {
      case Type::ARRAY:
        return a_.empty();
      case Type::OBJECT:
        return o_.empty();
      default:
        throw BadAccessException(
            "expected container type (array, object), got " + type(type_));
    }

    __builtin_unreachable();
  }

  // array
  template <typename T>
  typename std::enable_if_t<std::is_integral_v<T>, Value&> operator[](T i) {
    assert_type(Type::ARRAY);
    return a_[array_t::size_type(i)];
  }

  template <typename T>
  typename std::enable_if_t<std::is_integral_v<T>, const Value&> operator[](
      T i) const {
    assert_type(Type::ARRAY);
    return a_[array_t::size_type(i)];
  }

  void push_back(const Value& value) {
    if (type_ == Type::JSON_NULL) {
      destroy();
      ::new ((void*)(&a_)) array_t();
      type_ = Type::ARRAY;
    }

    assert_type(Type::ARRAY);
    a_.push_back(value);
  }

  void erase(array_t::const_iterator cit) {
    assert_type(Type::ARRAY);
    a_.erase(cit);
  }

  // object
  template <typename T>
  typename std::enable_if_t<std::is_convertible_v<T, std::string>, Value&>
  operator[](const T& key) {
    if (type_ == Type::JSON_NULL) {
      destroy();
      ::new ((void*)(&o_)) object_t();
      type_ = Type::OBJECT;
    }

    assert_type(Type::OBJECT);
    return o_[key];
  }

  template <typename T>
  typename std::enable_if_t<std::is_convertible_v<T, std::string>, const Value&>
  at(const T& key) const {
    assert_type(Type::OBJECT);
    return o_.at(key);
  }

  void insert(const std::string& key, const Value& value) {
    if (type_ == Type::JSON_NULL) {
      destroy();
      ::new ((void*)(&o_)) object_t();
      type_ = Type::OBJECT;
    }

    assert_type(Type::OBJECT);
    o_.insert({key, value});
  }

  void erase(const std::string& key) {
    assert_type(Type::OBJECT);
    o_.erase(key);
  }

  const Value& at(const char* key) const {
    assert_type(Type::OBJECT);
    return o_.at(key);
  }

  void insert(const char* key, const Value& value) {
    if (type_ == Type::JSON_NULL) {
      destroy();
      ::new ((void*)(&o_)) object_t();
      type_ = Type::OBJECT;
    }

    assert_type(Type::OBJECT);
    o_.insert({key, value});
  }

  template <class NullHandler, class BooleanHandler, class IntegralHandler,
            class DoubleHandler, class StringHandler, class ArrayHandler,
            class ObjectHandler>
  decltype(auto) visit(NullHandler&& null_fn, BooleanHandler&& boolean_fn,
                       IntegralHandler&& integral_fn, DoubleHandler&& double_fn,
                       StringHandler&& string_fn, ArrayHandler&& array_fn,
                       ObjectHandler&& object_fn) const {
    switch (type_) {
      case Type::JSON_NULL:
        return std::forward<NullHandler>(null_fn)();
      case Type::BOOLEAN:
        return std::forward<BooleanHandler>(boolean_fn)(b_);
      case Type::INTEGRAL:
        return std::forward<IntegralHandler>(integral_fn)(int32_t(n_));
      case Type::DOUBLE:
        return std::forward<DoubleHandler>(double_fn)(double(n_));
      case Type::STRING:
        return std::forward<StringHandler>(string_fn)(s_);
      case Type::ARRAY:
        return std::forward<ArrayHandler>(array_fn)(a_);
      case Type::OBJECT:
        return std::forward<ObjectHandler>(object_fn)(o_);
    }

    __builtin_unreachable();
  }

 private:
  enum Type { ARRAY, BOOLEAN, JSON_NULL, INTEGRAL, DOUBLE, OBJECT, STRING };

  void destroy() noexcept {
    switch (type_) {
      case Type::ARRAY:
        a_.~array_t();
        break;
      case Type::OBJECT:
        o_.~object_t();
        break;
      case Type::STRING:
        s_.std::string::~string();
        break;
      default:
        break;
    }

    type_ = Type::JSON_NULL;
  }

  void assert_type(Type expected) const {
    if (type_ != expected) {
      throw BadAccessException("expected type " + type(expected) + ", got " +
                               type(type_));
    }
  }

  std::string type(Type type) const {
    switch (type) {
      case ARRAY:
        return "array";
      case BOOLEAN:
        return "boolean";
      case JSON_NULL:
        return "null";
      case INTEGRAL:
      case DOUBLE:
        return "number";
      case OBJECT:
        return "object";
      case STRING:
        return "string";
    }

    __builtin_unreachable();
  }

  union {
    array_t a_;
    bool b_;
    double n_;
    object_t o_;
    std::string s_;
  };

  Type type_;
};

}  // namespace json
}  // namespace pulse

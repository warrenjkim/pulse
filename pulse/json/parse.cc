#include "pulse/json/parse.h"

#include <cctype>
#include <charconv>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/json/value.h"
#include "pulse/strings/cat.h"

namespace pulse::json {

namespace {

class Reader {
 public:
  explicit Reader(std::string_view json) : json_(json), pos_(0) {}

  bool eof() const { return pos_ >= json_.length(); }

  size_t tell() const { return pos_; }

  char peek() const { return json_[pos_]; }

  char get() { return json_[pos_++]; }

  std::string_view take() { return Substr(pos_++, 1); }

  bool Expect(char c) { return json_[pos_] == c && ++pos_; }

  template <std::predicate<char> Pred>
  bool Expect(Pred pred) {
    return pred(json_[pos_]) && ++pos_;
  }

  bool ExpectAnyOf(std::string_view chars) {
    return Expect(
        [chars](char c) { return chars.find(c) != std::string_view::npos; });
  }

  std::string_view Substr(size_t start,
                          std::optional<size_t> length = std::nullopt) const {
    return json_.substr(start,
                        length.has_value() ? *length : std::string::npos);
  }

 private:
  std::string_view json_;
  size_t pos_;
};

class Lexer {
 public:
  enum class TokenType {
    kObjectStart,
    kObjectEnd,
    kArrayStart,
    kArrayEnd,
    kComma,
    kColon,
    kString,
    kDouble,
    kIntegral,
    kBoolean,
    kNull,
    kEof,
  };

  struct Token {
    TokenType type;
    std::string_view value;
  };

  explicit Lexer(std::string_view json)
      : reader_(Reader(json)),
        curr_(Error{.code = pulse::Error::Code::kFailedPrecondition,
                    .message = "lexer not advanced yet"}) {}

  ~Lexer() = default;

  Lexer(Lexer&&) noexcept = default;
  Lexer& operator=(Lexer&&) noexcept = default;

  Lexer(const Lexer&) = delete;
  Lexer& operator=(const Lexer&) = delete;

  Lexer& operator++() {
    curr_ = NextToken();
    return *this;
  }

  const Token& operator*() const noexcept { return *curr_; }

  const Token* operator->() const noexcept { return &*curr_; }

  bool ok() const noexcept { return curr_.ok(); }

  Error error() const { return curr_.error(); }

  bool eof() const noexcept {
    return curr_.ok() && curr_->type == TokenType::kEof;
  }

 private:
  Result<Token> NextToken() {
    StripWhitespace();
    if (reader_.eof()) {
      return Token{.type = TokenType::kEof, .value = ""};
    }

    switch (reader_.peek()) {
      case 'n':
        return LexLiteral("null", TokenType::kNull);
      case 't':
        return LexLiteral("true", TokenType::kBoolean);
      case 'f':
        return LexLiteral("false", TokenType::kBoolean);
      case '"':
        return LexString();
      case '-':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        return LexNumber();
      case '[':
        return Token{.type = TokenType::kArrayStart, .value = reader_.take()};
      case ']':
        return Token{.type = TokenType::kArrayEnd, .value = reader_.take()};
      case '{':
        return Token{.type = TokenType::kObjectStart, .value = reader_.take()};
      case ':':
        return Token{.type = TokenType::kColon, .value = reader_.take()};
      case '}':
        return Token{.type = TokenType::kObjectEnd, .value = reader_.take()};
      case ',':
        return Token{.type = TokenType::kComma, .value = reader_.take()};
      default:
        return Error{.code = Error::Code::kInvalidArgument,
                     .message = strings::cat("unknown token at position ",
                                             reader_.tell())};
    }
  }

  Result<Token> LexLiteral(std::string_view literal, TokenType type) {
    size_t start = reader_.tell();
    for (char c : literal) {
      if (reader_.eof() || !reader_.Expect(c)) {
        return Error{
            .code = Error::Code::kInvalidArgument,
            .message = strings::cat("invalid literal at position ", start,
                                    ": expected '", literal, "'")};
      }
    }

    return Token{.type = type,
                 .value = reader_.Substr(start, reader_.tell() - start)};
  }

  Result<Token> LexString() {
    size_t start = reader_.tell();
    if (!reader_.Expect('"')) {
      return Error{
          .code = Error::Code::kInvalidArgument,
          .message = strings::cat("expected '\"' at position ", start)};
    }

    size_t content_start = reader_.tell();
    while (!reader_.eof()) {
      if (reader_.peek() == '\\') {
        if (!LexCtrl()) {
          return Error{
              .code = Error::Code::kInvalidArgument,
              .message = strings::cat("invalid control character at position ",
                                      reader_.tell())};
        }

        continue;
      }

      if (reader_.Expect('"')) {
        return Token{.type = TokenType::kString,
                     .value = reader_.Substr(
                         content_start, reader_.tell() - content_start - 1)};
      }

      (void)reader_.get();
    }

    return Error{
        .code = Error::Code::kInvalidArgument,
        .message = strings::cat("unterminated string at position ", start)};
  }

  bool LexCtrl() {
    if (!reader_.Expect('\\') || reader_.eof()) {
      return false;
    }

    switch (reader_.get()) {
      case 'u': {
        for (size_t i = 0; i < 4; i++) {
          if (reader_.eof() || !reader_.Expect(isxdigit)) {
            return false;
          }
        }

        return true;
      }
      case '"':
      case '\\':
      case '/':
      case 'b':
      case 'f':
      case 'n':
      case 'r':
      case 't':
        return true;
      default:
        return false;
    }
  }

  Result<Token> LexNumber() {
    size_t start = reader_.tell();

    Result<Token> integer = LexInteger();
    if (!integer.ok()) {
      return integer.error();
    }

    Result<Token> fraction = LexFraction();
    if (!fraction.ok()) {
      return fraction.error();
    }

    Result<Token> exponent = LexExponent();
    if (!exponent.ok()) {
      return exponent.error();
    }

    return Token{.type = (!fraction->value.empty() || !exponent->value.empty())
                             ? TokenType::kDouble
                             : TokenType::kIntegral,
                 .value = reader_.Substr(start, reader_.tell() - start)};
  }

  Result<Token> LexInteger() {
    size_t start = reader_.tell();
    reader_.Expect('-');
    if (reader_.eof() || !isdigit(reader_.peek())) {
      return Error{
          .code = Error::Code::kInvalidArgument,
          .message = strings::cat("invalid integer at position ", start)};
    }

    if (reader_.get() == '0' && !reader_.eof() && isdigit(reader_.peek())) {
      return Error{.code = Error::Code::kInvalidArgument,
                   .message = strings::cat("invalid integer at position ",
                                           start, ": leading zero")};
    }

    while (!reader_.eof() && reader_.Expect(isdigit));
    return Token{.type = TokenType::kIntegral,
                 .value = reader_.Substr(start, reader_.tell() - start)};
  }

  Result<Token> LexFraction() {
    size_t start = reader_.tell();
    if (reader_.eof() || !reader_.Expect('.')) {
      return Token{.type = TokenType::kIntegral, .value = ""};
    }

    if (reader_.eof() || !isdigit(reader_.peek())) {
      return Error{.code = Error::Code::kInvalidArgument,
                   .message = strings::cat("invalid fraction at position ",
                                           reader_.tell())};
    }

    while (!reader_.eof() && reader_.Expect(isdigit));

    return Token{.type = TokenType::kDouble,
                 .value = reader_.Substr(start, reader_.tell() - start)};
  }

  Result<Token> LexExponent() {
    size_t start = reader_.tell();
    if (reader_.eof() || !reader_.ExpectAnyOf("eE")) {
      return Token{.type = TokenType::kIntegral, .value = ""};
    }

    if (!reader_.eof()) {
      reader_.Expect([](char c) { return c == '+' || c == '-'; });
    }

    if (reader_.eof() || !isdigit(reader_.peek())) {
      return Error{.code = Error::Code::kInvalidArgument,
                   .message = strings::cat("invalid exponent at position ",
                                           reader_.tell())};
    }

    while (!reader_.eof() && reader_.Expect(isdigit));

    return Token{.type = TokenType::kIntegral,
                 .value = reader_.Substr(start, reader_.tell() - start)};
  }

  void StripWhitespace() { while (!reader_.eof() && reader_.Expect(isspace)); }

  Reader reader_;
  Result<Token> curr_;
};

uint32_t MakeCodePoint(const char c1, const char c2, const char c3,
                       const char c4) {
  auto to_uint32 = [](const char c) -> uint32_t {
    if (c >= '0' && c <= '9') {
      return uint32_t(c - '0');
    }

    if (c >= 'A' && c <= 'F') {
      return uint32_t(c - 'A' + 10);
    }

    return uint32_t(c - 'a' + 10);
  };

  uint32_t code_point = 0;
  code_point = (code_point << 4) | to_uint32(c1);
  code_point = (code_point << 4) | to_uint32(c2);
  code_point = (code_point << 4) | to_uint32(c3);
  code_point = (code_point << 4) | to_uint32(c4);

  return code_point;
}

// Section 3.8 Surrogates
// https://www.unicode.org/versions/Unicode15.0.0/ch03.pdf
Result<uint32_t> MakeSurrogatePair(std::string_view s, size_t* j) {
  *j += 6;
  // 0xD800 <= high-surrogate code point <= 0xDBFF
  uint32_t high_surrogate =
      MakeCodePoint(s[*j - 4], s[*j - 3], s[*j - 2], s[*j - 1]);
  if (0xD800 > high_surrogate || high_surrogate > 0xDBFF) {
    return high_surrogate;
  }

  if (*j + 5 >= s.length() || s[*j] != '\\' || s[*j + 1] != 'u') {
    return Error{
        .code = Error::Code::kInvalidArgument,
        .message = strings::cat("expected low surrogate after high surrogate: ",
                                s.substr(*j - 6, 6))};
  }

  *j += 6;
  // 0xDC00 <= low-surrogate code point <= 0xDFFF
  uint32_t low_surrogate =
      MakeCodePoint(s[*j - 4], s[*j - 3], s[*j - 2], s[*j - 1]);
  if (!(0xDC00 <= low_surrogate && low_surrogate <= 0xDFFF)) {
    return Error{.code = Error::Code::kInvalidArgument,
                 .message = strings::cat(
                     "invalid low surrogate ", s.substr(*j - 6, 6),
                     " after high surrogate: ", s.substr(*j - 12, 6))};
  }

  return 0x10000 + ((high_surrogate - 0xD800) << 10) + (low_surrogate - 0xDC00);
}

// https://www.ietf.org/rfc/rfc3629.txt
Result<void> EmbedUtf8(uint32_t code_point, std::string* res) {
  if (code_point > 0x10FFFF) {
    return Error{.code = Error::Code::kInvalidArgument,
                 .message = "code point out of unicode range (> 0x10FFFF)"};
  }

  if (0xD800 <= code_point && code_point <= 0xDFFF) {
    return Error{.code = Error::Code::kInvalidArgument,
                 .message = "code point in surrogate range [0xD800, 0xDFFF]"};
  }

  if (code_point < 0x80) {
    *res += (char)(code_point);
  } else if (code_point < 0x800) {
    *res += (char)(0xC0 | (code_point >> 6));
    *res += (char)(0x80 | (code_point & 0x3F));
  } else if (code_point < 0x10000) {
    *res += (char)(0xE0 | (code_point >> 12));
    *res += (char)(0x80 | ((code_point >> 6) & 0x3F));
    *res += (char)(0x80 | (code_point & 0x3F));
  } else {
    *res += (char)(0xF0 | (code_point >> 18));
    *res += (char)(0x80 | ((code_point >> 12) & 0x3F));
    *res += (char)(0x80 | ((code_point >> 6) & 0x3F));
    *res += (char)(0x80 | (code_point & 0x3F));
  }

  return Result<void>{};
}

Result<void> EmbedCtrl(char c, std::string* res) {
  switch (c) {
    case '"':
      *res += '"';
      break;
    case '\\':
      *res += '\\';
      break;
    case '/':
      *res += '/';
      break;
    case 'b':
      *res += '\b';
      break;
    case 'f':
      *res += '\f';
      break;
    case 'n':
      *res += '\n';
      break;
    case 'r':
      *res += '\r';
      break;
    case 't':
      *res += '\t';
      break;
    default:
      return Error{.code = Error::Code::kInvalidArgument,
                   .message = strings::cat("invalid escape character: ", c)};
  }

  return Result<void>{};
}

Result<void> ResolveEscapeSequences(std::string_view s, Value* value) {
  *value = std::string{};
  std::string* const res = &(value->as<std::string>());
  res->reserve(s.length());
  size_t i = 0;
  size_t j = 0;
  while (j < s.length()) {
    if (s[j] != '\\') {
      j++;
      continue;
    }

    if (s[j + 1] == 'u') {
      res->append(s, i, j - i);
      Result<uint32_t> code_point = MakeSurrogatePair(s, &j);
      if (!code_point.ok()) {
        return code_point.error();
      }

      if (Result<void> err = EmbedUtf8(*code_point, res); !err.ok()) {
        return err;
      }

      i = j;
    } else {
      res->append(s, i, j - i);
      if (Result<void> err = EmbedCtrl(s[++j], res); !err.ok()) {
        return err;
      }

      i = ++j;
    }
  }

  if (i < s.length()) {
    res->append(s, i, s.length() - i);
  }

  return Result<void>{};
}

Result<void> ParseValue(Lexer* lexer, Value* out);

Result<int64_t> ParseInt(std::string_view s) {
  int64_t result;
  auto [unused_ptr, ec] =
      std::from_chars(s.data(), s.data() + s.size(), result);
  if (ec != std::errc{}) {
    return Error{.code = Error::Code::kInvalidArgument,
                 .message = strings::cat("invalid integer: ", s)};
  }

  return result;
}

Result<double> ParseDouble(std::string_view s) {
  double result;
  auto [unused_ptr, ec] =
      std::from_chars(s.data(), s.data() + s.size(), result);
  if (ec != std::errc{}) {
    return Error{.code = Error::Code::kInvalidArgument,
                 .message = strings::cat("invalid double: ", s)};
  }

  return result;
}

Result<void> ParseArray(Lexer* lexer, Value* out) {
  *out = array_t{};
  array_t* array = &(out->as<array_t>());
  if (!(++(*lexer)).ok()) {
    return lexer->error();
  }

  if ((*lexer)->type == Lexer::TokenType::kArrayEnd) {
    if (!(++(*lexer)).ok()) {
      return lexer->error();
    }

    return Result<void>{};
  }

  while (true) {
    if (lexer->eof()) {
      return Error{.code = Error::Code::kInvalidArgument,
                   .message = "unterminated array"};
    }

    if (Result<void> err = ParseValue(lexer, &(array->emplace_back()));
        !err.ok()) {
      return err;
    }

    if (lexer->eof()) {
      return Error{.code = Error::Code::kInvalidArgument,
                   .message = "unterminated object"};
    }

    if ((*lexer)->type == Lexer::TokenType::kArrayEnd) {
      if (!(++(*lexer)).ok()) {
        return lexer->error();
      }
      return Result<void>{};
    }

    if ((*lexer)->type != Lexer::TokenType::kComma) {
      return Error{.code = Error::Code::kInvalidArgument,
                   .message = strings::cat("expected ',' or ']', got '",
                                           (*lexer)->value, "'")};
    }

    if (!(++(*lexer)).ok()) {
      return lexer->error();
    }
  }
}

Result<void> ParseObject(Lexer* lexer, Value* out) {
  *out = object_t{};
  object_t* object = &(out->as<object_t>());
  if (!(++(*lexer)).ok()) {
    return lexer->error();
  }

  if ((*lexer)->type == Lexer::TokenType::kObjectEnd) {
    if (!(++(*lexer)).ok()) {
      return lexer->error();
    }
    return Result<void>{};
  }

  while (true) {
    if (lexer->eof()) {
      return Error{.code = Error::Code::kInvalidArgument,
                   .message = "unterminated object"};
    }

    if ((*lexer)->type != Lexer::TokenType::kString) {
      return Error{.code = Error::Code::kInvalidArgument,
                   .message = strings::cat("expected string key, got '",
                                           (*lexer)->value, "'")};
    }

    Value key;
    if (Result<void> err = ResolveEscapeSequences((*lexer)->value, &key);
        !err.ok()) {
      return err;
    }

    if (!(++(*lexer)).ok()) {
      return lexer->error();
    }

    if ((*lexer)->type != Lexer::TokenType::kColon) {
      return Error{
          .code = Error::Code::kInvalidArgument,
          .message = strings::cat("expected ':', got '", (*lexer)->value, "'")};
    }

    if (!(++(*lexer)).ok()) {
      return lexer->error();
    }

    if (Result<void> err =
            ParseValue(lexer, &((*object)[std::move(key.as<std::string>())]));
        !err.ok()) {
      return err;
    }

    if (lexer->eof()) {
      return Error{.code = Error::Code::kInvalidArgument,
                   .message = "unterminated object"};
    }

    if ((*lexer)->type == Lexer::TokenType::kObjectEnd) {
      if (!(++(*lexer)).ok()) {
        return lexer->error();
      }

      return Result<void>{};
    }

    if ((*lexer)->type != Lexer::TokenType::kComma) {
      return Error{.code = Error::Code::kInvalidArgument,
                   .message = strings::cat("expected ',' or '}', got '",
                                           (*lexer)->value, "'")};
    }

    if (!(++(*lexer)).ok()) {
      return lexer->error();
    }
  }
}

Result<void> ParseValue(Lexer* lexer, Value* value) {
  switch ((*lexer)->type) {
    case Lexer::TokenType::kBoolean: {
      *value = (*lexer)->value == "true";
    } break;
    case Lexer::TokenType::kNull: {
      *value = nullptr;
    } break;
    case Lexer::TokenType::kString: {
      if (Result<void> err = ResolveEscapeSequences((*lexer)->value, value);
          !err.ok()) {
        return err;
      }
    } break;
    case Lexer::TokenType::kIntegral: {
      Result<int64_t> integer = ParseInt((*lexer)->value);
      if (!integer.ok()) {
        return integer.error();
      }

      *value = *integer;
    } break;
    case Lexer::TokenType::kDouble: {
      Result<double> fraction = ParseDouble((*lexer)->value);
      if (!fraction.ok()) {
        return fraction.error();
      }

      *value = *fraction;
    } break;
    case Lexer::TokenType::kArrayStart: {
      return ParseArray(lexer, value);
    }
    case Lexer::TokenType::kObjectStart: {
      return ParseObject(lexer, value);
    }
    default: {
      return Error{
          .code = Error::Code::kInvalidArgument,
          .message = strings::cat("unexpected token: '", (*lexer)->value, "'")};
    }
  }

  if (!(++(*lexer)).ok()) {
    return lexer->error();
  }

  return Result<void>{};
}

}  // namespace

Result<Value> Parse(std::string json) { return Parse(std::string_view(json)); }

Result<Value> Parse(std::string_view json) {
  Lexer lexer(json);
  ++lexer;
  if (!lexer.ok()) {
    return lexer.error();
  }

  Value out;
  if (Result<void> err = ParseValue(&lexer, &out); !err.ok()) {
    return err.error();
  }

  if (!lexer.eof()) {
    return Error{
        .code = Error::Code::kInvalidArgument,
        .message = strings::cat("unexpected token: '", lexer->value, "'")};
  }

  return out;
}

}  // namespace pulse::json

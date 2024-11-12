/**
 * \file parsing/parser.cpp
 **/
#include "parsing/parser.hpp"

#include <cstdio>
#include <ranges>
#include <string>

namespace other {

  void ParseContext::Cursor::update(char c) {
    /// TODO: customize this for different tab widths
    auto [o, r, col] = cursor_updater<2>()(offset, { row, column }, c);
    offset = o;
    row = r;
    column = col;
  }

  std::streambuf::int_type ParseContext::underflow() {
    return sbuf->sgetc();
  }

  std::streambuf::int_type ParseContext::uflow() {
    return last_read = sbuf->sbumpc();
  }
  // Note uflow() is not called for reading out eof.

  std::streampos ParseContext::seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which) {
    // Note istream(not streambuf) implements tellg() as seekoff(0, ios_base::cur).
    return sbuf->pubseekoff(off, way, which);
  }

  std::streampos ParseContext::seekpos(std::streampos pos, std::ios_base::openmode which) {
    return sbuf->pubseekpos(pos, which);
  }

  char CharacterParser::operator()(std::istream& stream) const {
    if (stream.fail()) {
      throw ParsingError();
    }

    const char c = stream.peek();
    if (c != '\0' && match(c)) {  // may possibly set eofbit
      stream.ignore();            // consume 'c'
      update_stream(stream);
      return c;
    }

    stream.setstate(std::ios::failbit);  // mark failure
    return '\0';                         // return 0 if not matched
  }

  bool MatchCharacter::match(char c) const {
    return c == ch;
  }

  Ref<Parser<char>> Char(char c) {
    return NewRef<MatchCharacter>(c);
  }

  bool MatchAny::match(char c) const {
    return true;
  }

  Ref<Parser<char>> AnyMatcher() {
    return NewRef<MatchAny>();
  }

  bool MatchNone::match(char c) const {
    return std::isspace(c);
  }

  Ref<Parser<char>> NoneMatcher() {
    return NewRef<MatchNone>();
  }

  bool MatchOneOf::match(char c) const {
    return chars.find(c) != std::string::npos;
  }

  Ref<Parser<char>> GroupMatcher(const std::string_view chars) {
    return NewRef<MatchOneOf>(chars);
  }

  bool MatchNoneOf::match(char c) const {
    return chars.find(c) == std::string::npos;
  }

  Ref<Parser<char>> ExcludeGroup(const std::string_view chars) {
    return NewRef<MatchNoneOf>(chars);
  }

  bool MatchAnyExcept::match(char c) const {
    return c != ch;
  }

  Ref<Parser<char>> AnyExceptMatcher(char c) {
    return NewRef<MatchAnyExcept>(c);
  }

  bool MatchNoneExcept::match(char c) const {
    return c == ch;
  }

  Ref<Parser<char>> NoneExceptMatcher(char c) {
    return NewRef<MatchNoneExcept>(c);
  }

  bool MatchFunction::match(char c) const {
    return func(c);
  }

  std::string ParseString::operator()(std::istream& stream) const {
    if (stream.fail()) {
      throw ParsingError();
    }

    MARK_OFFSET(stream);
    std::string result;
    for (const auto& c : str) {
      if (stream.peek() == c) {
        result.push_back(stream.get());
        update_stream(stream);
      } else {
        stream.setstate(std::ios::failbit);
        RETURN_OR_WEAK_FAILURE(stream, result);
      }
    }

    return result;
  }

  std::string ParseAllUntil::operator()(std::istream& stream) const {
    if (stream.fail()) {
      throw ParsingError();
    }

    std::string result;
    while (stream.peek() != ch && !stream.eof()) {
      result.push_back(stream.get());
      update_stream(stream);
    }

    return result;
  }

  std::string ParseAllUntilThenTake::operator()(std::istream& stream) const {
    if (stream.fail()) {
      throw ParsingError();
    }

    std::string result;
    while (stream.peek() != ch && !stream.eof()) {
      result.push_back(stream.get());
      update_stream(stream);
    }

    if (stream.eof()) {
      throw ParsingError();
    }

    result.push_back(stream.get());
    update_stream(stream);

    return result;
  }

  Ref<Parser<std::string>> ParseUntil(char c) {
    return NewRef<ParseAllUntil>(c);
  }

  Ref<Parser<std::string>> ParseUntilThenTake(char c) {
    return NewRef<ParseAllUntilThenTake>(c);
  }

  Ref<Parser<char>> MatchBlank() {
    MatchFunction::matcher_fn blank = &std::isblank;
    return NewRef<MatchFunction>(blank);
  }

  Ref<Parser<char>> MatchAlpha() {
    MatchFunction::matcher_fn alpha = &std::isalpha;
    return NewRef<MatchFunction>(alpha);
  }

  Ref<Parser<char>> MatchDigit() {
    MatchFunction::matcher_fn digit = &std::isdigit;
    return NewRef<MatchFunction>(digit);
  }

  Ref<Parser<char>> MatchAlnum() {
    MatchFunction::matcher_fn alnum = &std::isalnum;
    return NewRef<MatchFunction>(alnum);
  }

  Ref<Parser<char>> MatchWhitespace() {
    MatchFunction::matcher_fn whitespace = &std::isspace;
    return NewRef<MatchFunction>(whitespace);
  }

  Ref<Parser<std::string>> MatchAnyString() {
    return Many<std::string>(AnyMatcher());
  }

  Ref<Parser<std::string>> MatchAnyStringWithout(const std::string_view chars) {
    return Many<std::string>(ExcludeGroup(chars));
  }

  Ref<Parser<std::string>> MatchString(const std::string_view str) {
    Ref<Parser<std::string>> parser = +Char(str[0]);
    for (size_t i = 1; i < str.size(); ++i) {
      parser = parser + (+Char(str[i]));
    }
    return parser;
  }

  namespace {

    static std::istream& TrimBeginning(std::istream& stream) {
      while (!stream.eof() && std::isspace(stream.peek())) {
        stream.ignore();
      }
      return stream;
    }

    static std::string TrimEnd(const std::string_view str) {
      return str |
        std::views::filter([](char c) { return !std::isspace(c); }) |
        std::ranges::to<std::string>();
    }

    static std::string StripParens(const std::string_view str) {
      return str |
        std::views::filter([](char c) { return c != '{' && c != '}'; }) |
        std::views::filter([](char c) { return c != '[' && c != ']'; }) |
        std::views::filter([](char c) { return c != '(' && c != ')'; }) |
        std::ranges::to<std::string>();
    }

    constexpr inline std::pair<std::istream& (*)(std::istream&), std::string (*)(const std::string_view)> trim_whitespace{ &TrimBeginning, &TrimEnd };

  }  // anonymous namespace

  Ref<Parser<std::string>> MatchAndTrim(const std::string_view str) {
    return (MatchString(str) | +MatchWhitespace()) | std::pair{ &TrimBeginning, &TrimEnd };
  }

  Ref<Parser<std::string>> MatchAndStripParens(const std::string_view str) {
    return MatchAndTrim(str) | &StripParens;
  }

  Ref<Parser<std::string>> MatchIdentifier() {
    return MatchAnyStringWithout("\n\t\r") | trim_whitespace;
  }

  Ref<Parser<std::string>> MatchIdentifierAndStripParens() {
    return (MatchIdentifier() | trim_whitespace) | &StripParens;
  }

  std::string ConcatParser::operator()(std::istream& stream) const {
    if (stream.fail()) {
      throw ParsingError();
    }

    MARK_OFFSET(stream);
    std::string result{ (*parser1)(stream) };
    if (stream.fail()) {
      RETURN_OR_WEAK_FAILURE(stream, result);
    }

    result.append((*parser2)(stream));
    if (stream.fail()) {
      RETURN_OR_WEAK_FAILURE(stream, result);
    }

    return result;
  }

  std::string char_to_string(char c) {
    return std::string(1, c);
  }

  Ref<Parser<std::string>> operator+(const Ref<Parser<char>>& parser) {
    return parser | &char_to_string;
  }

  Ref<Parser<std::string>> operator+(const Ref<Parser<std::string>>& parser1, const Ref<Parser<std::string>>& parser2) {
    return NewRef<ConcatParser>(parser1, parser2);
  }

  Ref<Parser<std::string>> operator+(const Ref<Parser<std::string>>& parser1, const Ref<Parser<char>>& parser2) {
    return parser1 + (+parser2);
  }

  Ref<Parser<std::string>> operator+(const Ref<Parser<char>>& parser1, const Ref<Parser<std::string>>& parser2) {
    return +parser1 + parser2;
  }

  Ref<Parser<std::string>> operator+(const Ref<Parser<char>>& parser1, const Ref<Parser<char>>& parser2) {
    return +parser1 + +parser2;
  }

  Ref<Parser<std::vector<std::string>>> SplitStringOn(char delim) {
    Ref<Parser<std::string>> string_matcher = MatchAnyStringWithout(std::string(1, delim)) | trim_whitespace;
    return Split<std::vector<std::string>>(string_matcher, Char(delim));
  }

  void ParseEof::operator()(std::istream& stream) const {
    if (stream.fail()) {
      throw ParsingError();
    }

    if (!(stream.eof() || stream.peek() == EOF)) {
      stream.setstate(std::ios::failbit);
    }
  }

  Ref<Parser<void>> EndOfFile() {
    return NewRef<ParseEof>();
  }

  Ref<Parser<void>> SkipAny() {
    return Skip(AnyMatcher());
  }

  Ref<Parser<void>> Skip(char c) {
    return Skip(Char(c));
  }

  void SkipString::operator()(std::istream& stream) const {
    if (stream.fail()) {
      throw ParsingError();
    }

    if (str.empty()) {
      return;
    }

    MARK_OFFSET(stream);
    for (const auto& c : str) {
      if (stream.peek() == c) {
        stream.ignore();
        update_stream(stream);
      } else {
        stream.setstate(std::ios::failbit);
        RETURN_OR_WEAK_FAILURE(stream);
      }
    }
  }

  Ref<Parser<void>> Skip(const std::string_view str) {
    return NewRef<SkipString>(str);
  }

  void SkipAllUntil::operator()(std::istream& stream) const {
    if (stream.fail()) {
      throw ParsingError();
    }

    while (!stream.eof() && stream.peek() != ch) {
      stream.ignore();
      update_stream(stream);
    }
  }

  Ref<Parser<void>> SkipUntil(char c) {
    return NewRef<SkipAllUntil>(Char(c));
  }

}  // namespace other
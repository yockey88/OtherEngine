/**
 * \file parsing/parser_combinators.hpp
 **/
#ifndef OTHER_ENGINE_PARSER_COMBINATORS_HPP
#define OTHER_ENGINE_PARSER_COMBINATORS_HPP

#include <concepts>
#include <istream>
#include <sstream>
#include <streambuf>
#include <string>
#include <string_view>
#include <utility>

#include "core/meta.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"

namespace other {

  std::istream& TrimBeginning(std::istream& stream);
  std::string TrimEnd(const std::string_view str);
  std::string TrimBeginningAndEnd(const std::string& str);
  std::string StripParens(const std::string_view str);

  class ParseContext : public std::streambuf {
   private:
    template <size_t TW>
    struct cursor_updater {
      std::tuple<std::streamoff, size_t, size_t> operator()(std::streamoff offset, std::pair<size_t, size_t> cursor, char c) {
        cursor.first++;
        if (c == '\t') {
          cursor.second += TW;
        } else {
          cursor.second++;
        }
        return { offset + 1, cursor.first, cursor.second };
      }
    };

   public:
    ParseContext(std::streambuf* sbuf) : sbuf(sbuf) {}

    struct Cursor {
      std::streamoff offset;
      size_t row = 1, column = 1;
      void update(char c);
    } curs;

    char last_read = 0;

   protected:
    std::streambuf* const sbuf;

    std::streambuf::int_type underflow();
    std::streambuf::int_type uflow();

    std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
    std::streampos seekpos(std::streampos pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
  };

#define MARK_OFFSET(stream) std::streamoff _off(tellg(stream, 0))

#define RETURN_TO_MARK(stream)              \
  do {                                      \
    stream.clear();                         \
    stream.seekg(_off, std::ios_base::beg); \
  } while (false)

#define CHECK_STREAM(stream)                  \
  if (stream.fail() && tellg(stream, _off)) { \
    throw ParsingError();                     \
  } else

#define RETURN_OR_WEAK_FAILURE(stream, ...)          \
  do {                                               \
    if (stream.fail() && tellg(stream, _off) == 0) { \
      throw ParsingError();                          \
    }                                                \
    return __VA_ARGS__;                              \
  } while (0)

#define RETURN_IF_FAIL(stream, ...) \
  if (stream.fail()) {              \
    if (tellg(stream, _off) == 0) { \
      throw ParsingError();         \
    }                               \
    return __VA_ARGS__;             \
  } else

  // typing savers for static_cast<pos_stream *>(s.rdbuf())
  inline void update_stream(std::istream& stream) {
    ParseContext* parse_stream = static_cast<ParseContext*>(stream.rdbuf());
    parse_stream->curs.update(parse_stream->last_read);
  }

  // similar to s.tellg() but also work for istreams that disables it
  inline std::streamoff tellg(std::istream& s, std::streamoff since) {
    return static_cast<ParseContext*>(s.rdbuf())->curs.offset - since;
  }

  inline ParseContext::Cursor& stream_position(std::istream& s) {
    return static_cast<ParseContext*>(s.rdbuf())->curs;
  }

  template <typename T>
  struct Parser : public RefCounted {
    virtual ~Parser() = default;
    virtual T operator()(std::istream& stream) const = 0;
  };

  template <typename T>
  inline T operator>>(std::istream& stream, const Ref<Parser<T>>& parser) {
    return (*parser)(stream);
  }

  template <>
  inline void operator>>(std::istream& stream, const Ref<Parser<void>>& parser) {
    (*parser)(stream);
  }

  struct CharacterParser : Parser<char> {
    virtual ~CharacterParser() = default;
    char operator()(std::istream& stream) const override;

   protected:
    virtual char update(std::istream& stream) const;
    virtual bool match(char c) const = 0;
  };

  struct MatchCharacter : CharacterParser {
    const char ch;

    MatchCharacter(char c)
        : ch(c) {}

    bool match(char c) const override;
  };
  Ref<Parser<char>> Char(char c);

  struct MatchAny : CharacterParser {
    bool match(char c) const override;
  };
  Ref<Parser<char>> AnyMatcher();

  struct MatchNone : CharacterParser {
    bool match(char c) const override;
  };
  Ref<Parser<char>> NoneMatcher();

  struct MatchOneOf : CharacterParser {
    const std::string chars;

    MatchOneOf(const std::string_view chars)
        : chars(chars) {}

    bool match(char c) const override;
  };
  Ref<Parser<char>> GroupMatcher(const std::string_view chars);

  struct MatchNoneOf : CharacterParser {
    const std::string chars;

    MatchNoneOf(const std::string_view chars)
        : chars(chars) {}

    bool match(char c) const override;
  };
  Ref<Parser<char>> ExcludeGroup(const std::string_view chars);

  struct MatchAnyExcept : CharacterParser {
    const char ch;

    MatchAnyExcept(char c)
        : ch(c) {}

    bool match(char c) const override;
  };
  Ref<Parser<char>> AnyExceptMatcher(char c);

  struct MatchNoneExcept : CharacterParser {
    const char ch;

    MatchNoneExcept(char c)
        : ch(c) {}

    bool match(char c) const override;
  };
  Ref<Parser<char>> NoneExceptMatcher(char c);

  struct MatchFunction : CharacterParser {
    using matcher_fn = int (*const)(int);
    matcher_fn func;

    MatchFunction(matcher_fn fn)
        : func(fn) {}

    bool match(char c) const override;
  };

  struct ParseString : Parser<std::string> {
    const std::string str;

    ParseString(const std::string_view str)
        : str(str) {}

    std::string operator()(std::istream& stream) const override;
  };

  struct ParseAllUntil : Parser<std::string> {
    std::string chars;

    ParseAllUntil(char c)
        : chars({ c }) {}

    ParseAllUntil(const std::string_view chars)
        : chars(chars) {}

    std::string operator()(std::istream& stream) const override;
  };

  struct ParseAllUntilThenTake : Parser<std::string> {
    std::string chars;

    ParseAllUntilThenTake(char c)
        : chars({ c }) {}

    ParseAllUntilThenTake(const std::string_view chars)
        : chars(chars) {}

    std::string operator()(std::istream& stream) const override;
  };

  Ref<Parser<void>> SkipSpaces();

  Ref<Parser<std::string>> ParseUntil(char c);
  Ref<Parser<std::string>> ParseUntilThenTake(char c);
  Ref<Parser<std::string>> ParseUntil(const std::string_view chars);
  Ref<Parser<std::string>> ParseUntilThenTake(const std::string_view chars);

  Ref<Parser<char>> MatchBlank();
  Ref<Parser<char>> MatchAlpha();
  Ref<Parser<char>> MatchDigit();
  Ref<Parser<char>> MatchAlnum();
  Ref<Parser<char>> MatchWhitespace();
  Ref<Parser<std::string>> MatchAnyString();
  Ref<Parser<std::string>> MatchAnyStringWithout(const std::string_view chars);
  Ref<Parser<std::string>> MatchString(const std::string_view str);

  Ref<Parser<std::string>> SkipWhitespaceThenMatch(const std::string_view str);
  Ref<Parser<std::string>> MatchAndTrim(const std::string_view str);
  Ref<Parser<std::string>> MatchAndStripParens(const std::string_view str);
  Ref<Parser<std::string>> MatchIdentifier();
  Ref<Parser<std::string>> MatchIdentifierAndStripParens();

  struct ParseEof : Parser<void> {
    void operator()(std::istream& stream) const override;
  };

  Ref<Parser<void>> EndOfFile();

  template <typename T>
  struct SkipParser : Parser<void> {
    const Ref<Parser<T>> matcher;

    SkipParser(const Ref<Parser<T>>& matcher)
        : matcher(matcher) {}

    void operator()(std::istream& stream) const override {
      (void)(*matcher)(stream);  /// matcher will iterate stream if need be
    }
  };

  Ref<Parser<void>> SkipAny();

  template <typename T>
  Ref<Parser<void>> Skip(const Ref<Parser<T>>& matcher) {
    return NewRef<SkipParser<T>>(matcher);
  }

  Ref<Parser<void>> Skip(char c);

  struct SkipString : Parser<void> {
    const std::string str;

    SkipString(const std::string_view str)
        : str(str) {}

    void operator()(std::istream& stream) const override;
  };

  Ref<Parser<void>> Skip(const std::string_view str);

  struct SkipAllUntil : Parser<void> {
    char ch;

    SkipAllUntil(char c)
        : ch(c) {}

    void operator()(std::istream& stream) const override;
  };

  Ref<Parser<void>> SkipUntil(char c);

  struct SkipAllWhile : Parser<void> {
    const Ref<Parser<char>> matcher;

    SkipAllWhile(const Ref<Parser<char>>& matcher)
        : matcher(matcher) {}

    void operator()(std::istream& stream) const override;
  };

  Ref<Parser<void>> SkipWhile(const Ref<Parser<char>>& matcher);

  template <typename T1, typename F>
  concept ParserMapFn =
    requires(const Parser<T1>& p1, F map) {
      { p1(std::declval<std::istream&>()) } -> std::same_as<T1>;
      { map(p1(std::declval<std::istream&>())) } -> std::convertible_to<std::invoke_result_t<F, T1>>;
    };

  template <typename T1, typename F>
    requires ParserMapFn<T1, F>
  struct ParserMap : Parser<std::invoke_result_t<F, T1>> {
    using R = std::invoke_result_t<F, T1>;

    const Ref<Parser<T1>> parser;
    F map;

    ParserMap(const Ref<Parser<T1>>& parser, F map)
        : parser(parser), map(map) {}

    R operator()(std::istream& stream) const override {
      T1 val = (*parser)(stream);
      if (stream.fail()) {
        return R{};
      }
      return map(val);
    }
  };

  template <typename F>
  struct ParserMap<void, F> : Parser<std::invoke_result_t<F>> {
    using R = std::invoke_result_t<F>;

    const Ref<Parser<void>> parser;
    F map;

    ParserMap(const Ref<Parser<void>>& parser, F map)
        : parser(parser), map(map) {}

    R operator()(std::istream& stream) const override {
      (*parser)(stream);
      if (stream.fail()) {
        return R{};
      }
      return map();
    }
  };

  template <typename T1, typename F>
    requires ParserMapFn<T1, F>
  inline Ref<Parser<std::invoke_result_t<F, T1>>> operator|(const Ref<Parser<T1>>& parser, F map) {
    return NewRef<ParserMap<T1, F>>(parser, map);
  }

  struct ConcatParser : Parser<std::string> {
    const Ref<Parser<std::string>> parser1;
    const Ref<Parser<std::string>> parser2;

    ConcatParser(const Ref<Parser<std::string>>& parser1, const Ref<Parser<std::string>>& parser2)
        : parser1(parser1), parser2(parser2) {}

    std::string operator()(std::istream& stream) const override;
  };

  std::string char_to_string(char c);
  Ref<Parser<std::string>> Str(const Ref<Parser<char>>& parser);

  Ref<Parser<std::string>> operator+(const Ref<Parser<std::string>>& parser1, const Ref<Parser<std::string>>& parser2);
  Ref<Parser<std::string>> operator+(const Ref<Parser<std::string>>& parser1, const Ref<Parser<char>>& parser2);
  Ref<Parser<std::string>> operator+(const Ref<Parser<char>>& parser1, const Ref<Parser<std::string>>& parser2);
  Ref<Parser<std::string>> operator+(const Ref<Parser<char>>& parser1, const Ref<Parser<char>>& parser2);

  template <typename T>
    requires is_container<T>
  struct ParseMany : Parser<T> {
    /// apply this parser many times
    const Ref<Parser<typename T::value_type>> p;

    ParseMany(const Ref<Parser<typename T::value_type>>& parser)
        : p(parser) {}

    T operator()(std::istream& stream) const override {
      using val_t = typename T::value_type;

      T result;
      while (!stream.eof() && stream.peek() != '\0') {
        val_t val = (*p)(stream);
        if (stream.fail()) {
          stream.clear();
          /// force copy
          return T{ result };
        }

        result.insert(result.end(), val);
      }

      if (stream.eof()) {
        stream.setstate(std::ios::failbit);
      }

      return result;
    }
  };

  template <typename T>
    requires not_container<T>
  struct SkipMany : Parser<void> {
    const Ref<Parser<void>> p;

    SkipMany(const Ref<Parser<T>>& parser)
        : p(parser) {}

    void operator()(std::istream& stream) const {
      while (!stream.eof()) {
        (*p)(stream);
        if (stream.fail()) {
          stream.clear();
          return;
        }
      }
    }
  };

  template <typename T>
  Ref<Parser<T>> Many(const Ref<Parser<typename T::value_type>>& parser) {
    return NewRef<ParseMany<T>>(parser);
  }

  template <typename T>
    requires(is_container<T> || std::is_same_v<T, std::string>)
  struct ParseOneOrMore : Parser<T> {
    const Ref<Parser<typename T::value_type>> p;

    ParseOneOrMore(const Ref<Parser<typename T::value_type>>& parser)
        : p(parser) {}

    T operator()(std::istream& stream) const override {
      using val_t = typename T::value_type;

      T result;
      while (!stream.eof()) {
        val_t val = (*p)(stream);
        if (stream.fail()) {
          stream.clear();
          if (result.empty()) {
            return T{};
          }
          return T{ result };
        }

        result.insert(result.end(), val);
      }
      return result;
    }
  };

  template <typename T>
    requires(is_container<T> || std::is_same_v<T, std::string>)
  Ref<Parser<T>> OneOrMore(const Ref<Parser<typename T::value_type>>& parser) {
    return NewRef<ParseOneOrMore<T>>(parser);
  }

  template <typename T1, typename T2>
  struct ParserSequence : Parser<T2> {
    const Ref<Parser<T1>> parser1;
    const Ref<Parser<T2>> parser2;

    ParserSequence(const Ref<Parser<T1>>& parser1, const Ref<Parser<T2>>& parser2)
        : parser1(parser1), parser2(parser2) {}

    T2 operator()(std::istream& stream) const override {
      MARK_OFFSET(stream);

      if constexpr (std::same_as<void, T1>) {
        (*parser1)(stream);
        if (stream.fail()) {
          return T2{};
        }

        T2 val = (*parser2)(stream);
        RETURN_OR_WEAK_FAILURE(stream, val);
      } else if constexpr (std::same_as<void, T2>) {
        T1 val = (*parser1)(stream);
        if (stream.fail()) {
          return T2{};
        }

        (*parser2)(stream);
        RETURN_OR_WEAK_FAILURE(stream, T2{ val });
      } else if constexpr (is_container<T2> || std::same_as<T2, std::string>) {
        if constexpr (std::same_as<T2, std::string>) {
          static_assert(std::same_as<T1, char> || std::same_as<T1, std::string>, "Cannot append different types to string");
          T2 val = T2{ (*parser1)(stream) };
          if (stream.fail()) {
            return T2{};
          }

          T2 res = (*parser2)(stream);
          val.append(res);

          RETURN_OR_WEAK_FAILURE(stream, val);
        } else if (is_container<T2>) {
          static_assert(std::same_as<T1, typename T2::value_type>, "Cannot append different types to container");
          T2 val = T2{ (*parser1)(stream) };
          if (stream.fail()) {
            return T2{};
          }

          T2 res = (*parser2)(stream);
          val.insert(val.end(), res.begin(), res.end());

          RETURN_OR_WEAK_FAILURE(stream, val);
        } else {
          T1 val = (*parser1)(stream);
          if (stream.fail()) {
            return T2{};
          }

          T2 res = (*parser2)(stream);
          RETURN_OR_WEAK_FAILURE(stream, res);
        }
      } else if constexpr (
        requires { { T2{ std::declval<T1>() } }; { std::declval<T2>().append(std::declval<T1>()) }; } ||
        requires { { T2{ std::declval<T1>() } }; { std::declval<T2>().insert(std::declval<T2>().end(), std::declval<T1>()) }; } ||
        requires { { T2{ std::declval<T1>() } }; { std::declval<T2>().insert(std::declval<T2>().end(), std::declval<T2>().begin(), std::declval<T2>().end()) }; }
      ) {
        T2 val = T2{ (*parser1)(stream) };
        if (stream.fail()) {
          return T2{};
        }

        T2 res = (*parser2)(stream);
        if constexpr (requires { val.append(res); }) {
          val.append(res);
        } else if constexpr (requires { val.insert(val.end(), res.begin(), res.end()); }) {
          val.insert(val.end(), res.begin(), res.end());
        } else {
          val.insert(val.end(), res.begin(), res.end());
        }
        return val;
      } else {
        T1 val = (*parser1)(stream);
        if (stream.fail()) {
          return T2{};
        }

        T2 res = (*parser2)(stream);
        RETURN_OR_WEAK_FAILURE(stream, res);
      }
    }
  };

  template <>
  struct ParserSequence<void, void> : Parser<void> {
    const Ref<Parser<void>> parser1;
    const Ref<Parser<void>> parser2;

    ParserSequence(const Ref<Parser<void>>& parser1, const Ref<Parser<void>>& parser2)
        : parser1(parser1), parser2(parser2) {}

    void operator()(std::istream& stream) const override {
      (*parser1)(stream);
      (*parser2)(stream);
    }
  };

  template <typename T1, typename T2>
  inline Ref<Parser<T2>> Seq(const Ref<Parser<T1>>& parser1, const Ref<Parser<T2>>& parser2) {
    return NewRef<ParserSequence<T1, T2>>(parser1, parser2);
  }

  template <typename T1, typename T2>
  inline Ref<Parser<T2>> operator>>(const Ref<Parser<T1>>& parser1, const Ref<Parser<T2>>& parser2) {
    return NewRef<ParserSequence<T1, T2>>(parser1, parser2);
  }

  template <typename T>
  concept NotVoid = !std::same_as<T, void>;

  template <typename... Ts>
  concept NoneVoid = (NotVoid<Ts> && ...);

  template <typename F, typename T>
  concept FunctionInvocableWithParser = requires(F f, Ref<Parser<T>> p) {
    f(p);
  };

  template <typename F, typename... Ts>
  concept ApplicableToAll = (FunctionInvocableWithParser<F, Ts> && ...);

  template <typename... Ts>
  using ParserTuple = std::tuple<Ref<Parser<Ts>>...>;

  template <size_t I = 0, typename F, typename... Ts>
    requires(I == sizeof...(Ts))
  decltype(auto) Apply(const ParserTuple<Ts...>&, F func) {
    return std::make_tuple();
  }

  template <size_t I = 0, typename F, typename... Ts>
    requires(I < sizeof...(Ts))
  decltype(auto) Apply(const ParserTuple<Ts...>& parsers, F func) {
    auto argi = std::make_tuple(func(std::get<I>(parsers)));
    auto rest = Apply<I + 1, F, Ts...>(parsers, func);
    return std::tuple_cat(argi, rest);
  }

  template <typename... Ts>
  struct MultiParser : Parser<std::tuple<Ts...>> {
    const ParserTuple<Ts...> parsers;

    MultiParser(const Ref<Parser<Ts>>&... parsers)
        : parsers(std::make_tuple(parsers...)) {}

    std::tuple<Ts...> operator()(std::istream& stream) const override {
      auto evaluate = [&stream](auto& parser) -> decltype((*parser)(stream)) {
        return (*parser)(stream);
      };
      return Apply<0, decltype(evaluate), Ts...>(parsers, evaluate);
    }
  };

  template <typename... Ts>
  Ref<Parser<std::tuple<Ts...>>> ParseMultiple(const Ref<Parser<Ts>>&... parsers) {
    return NewRef<MultiParser<Ts...>>(parsers...);
  }

  template <typename C, typename T>
    requires is_container<C> && is_container<typename C::value_type> && std::same_as<typename C::value_type::value_type, T>
  struct SplitOnDelimiter : public Parser<C> {
    using val_t = typename C::value_type;

    const Ref<Parser<val_t>> val_parser;
    const Ref<Parser<T>> delim_parser;

    SplitOnDelimiter(const Ref<Parser<val_t>>& val_parser, const Ref<Parser<T>>& delim_parser)
        : val_parser(val_parser), delim_parser(delim_parser) {}

    C operator()(std::istream& stream) const override {
      C result{};
      val_t val = (*val_parser)(stream);
      if (stream.fail()) {
        stream.clear();
        return result;
      }

      result.insert(result.end(), val);
      do {
        (*delim_parser)(stream);
        if (stream.fail()) {
          stream.clear();
          return result;
        }

        val = (*val_parser)(stream);
        if (stream.fail()) {
          stream.clear();
          return result;
        }

        result.insert(result.end(), val);
      } while (true);

      stream.clear();
      return result;
    }
  };

  template <typename C, typename T>
    requires is_container<C> && is_container<typename C::value_type> && std::same_as<typename C::value_type::value_type, T>
  Ref<Parser<C>> Split(const Ref<Parser<typename C::value_type>>& val_parser, const Ref<Parser<T>>& delim_parser) {
    return NewRef<SplitOnDelimiter<C, T>>(val_parser, delim_parser);
  }

  Ref<Parser<std::vector<std::string>>> SplitStringOn(char delim);

  template <typename T1, typename T2>
  concept ReplacableWith =
    (std::same_as<T1, void> && std::same_as<T2, void>) ||
    requires(const Ref<Parser<T1>>& p1, const Ref<Parser<T2>>& p2) {
      T1{ (*p1)(std::declval<std::istream&>()) };
    };

  template <typename T1, typename T2>
    requires ReplacableWith<T1, T2>
  struct ParseOr : Parser<T1> {
    const Ref<Parser<T1>> parser;
    const Ref<Parser<T2>> fallback;

    ParseOr(const Ref<Parser<T1>>& parser, const Ref<Parser<T2>>& fallback)
        : parser(parser), fallback(fallback) {}

    T1 operator()(std::istream& stream) const override {
      if (stream.fail()) {
        return T1{};
      }

      T1 val = (*parser)(stream);
      if (!stream.fail()) {
        return val;
      } else if (stream.eof()) {
        return T1{};
      }
      stream.clear();

      T2 val2 = (*fallback)(stream);
      if (!stream.fail()) {
        return T1{ val2 };
      } else if (stream.eof()) {
        return T1{};
      }

      throw ParsingError();
    }
  };

  template <>
  struct ParseOr<void, void> : Parser<void> {
    const Ref<Parser<void>> parser;
    const Ref<Parser<void>> fallback;

    ParseOr(const Ref<Parser<void>>& parser, const Ref<Parser<void>>& fallback)
        : parser(parser), fallback(fallback) {}

    void operator()(std::istream& stream) const override {
      if (stream.fail()) {
        return;
      }

      (*parser)(stream);
      if (!stream.fail()) {
        return;
      }

      stream.clear();

      (*fallback)(stream);
      if (stream.fail()) {
        throw ParsingError();
      }

      return;
    }
  };

  template <typename T1, typename T2>
    requires ReplacableWith<T1, T2>
  Ref<Parser<T2>> Or(const Ref<Parser<T1>>& parser, const Ref<Parser<T2>>& fallback) {
    return NewRef<ParseOr<T1, T2>>(parser, fallback);
  }

  template <typename T1, typename T2>
    requires ReplacableWith<T1, T2>
  Ref<Parser<T1>> operator|(const Ref<Parser<T1>>& parser, const Ref<Parser<T2>>& fallback) {
    return Or(parser, fallback);
  }

  template <typename T1, typename F>
  concept ParserFilterFn =
    requires(const Parser<T1>& p1, F filter) {
      { filter(std::declval<std::istream&>()) } -> std::same_as<std::istream&>;
      { p1(filter(std::declval<std::istream&>())) } -> std::same_as<T1>;
    };

  template <typename T, typename F>
  struct FilterThen : Parser<T> {
    const Ref<Parser<T>> parser;
    F filter;

    FilterThen(const Ref<Parser<T>>& parser, F filter)
        : parser(parser), filter(filter) {}

    T operator()(std::istream& stream) const override {
      std::istream& in = filter(stream);

      T val = (*parser)(in);
      if (stream.fail()) {
        return T{};
      }

      return val;
    }
  };

  template <typename T, typename F>
    requires ParserFilterFn<T, F>
  Ref<Parser<T>> operator|(F filter, const Ref<Parser<T>>& parser) {
    return NewRef<FilterThen<T, F>>(parser, filter);
  }

  template <typename T, typename F, typename M>
    requires ParserFilterFn<T, F> && ParserMapFn<T, M>
  Ref<Parser<T>> FilterThenMap(const Ref<Parser<T>>& parser, F filter, M map) {
    auto filt = filter | parser;
    return filt | map;
  }

  template <typename T, typename F, typename M>
    requires ParserFilterFn<T, F> && ParserMapFn<T, M>
  Ref<Parser<std::invoke_result_t<M, T>>> operator|(const Ref<Parser<T>>& parser, std::pair<F, M> filter_map) {
    return FilterThenMap(parser, filter_map.first, filter_map.second);
  }

  template <typename C1, typename C2>
  concept SimilarContainers = is_container<C1> && is_container<C2> && std::same_as<typename C1::value_type, typename C2::value_type>;

  template <typename C, typename C1, typename C2>
  concept NestableParsers = SimilarContainers<C1, C2> && is_container<C> && std::same_as<typename C::value_type, C2>;

  template <typename OC, typename C1, typename C2>
    requires NestableParsers<OC, C1, C2> &&
    //// can we construct on istream??
    requires(typename C1::value_type val) {
      std::istringstream{ val };
    }
  Ref<Parser<OC>> ForEach(const Ref<Parser<C1>>& parser, const Ref<Parser<C2>>& for_each) {
    return parser | [=](const C1& c1) -> OC {
      OC result;
      for (const auto& data : c1) {
        std::istringstream stream(data);
        C2 val = (*for_each)(stream);
        if (stream.fail()) {
          return result;
        }

        result.insert(result.end(), val);
      }
      return result;
    };
  }

  template <typename C, typename T>
  concept CanStore =
    is_container<C> &&
    (std::same_as<typename C::value_type, T> || std::convertible_to<T, typename C::value_type>);

  template <typename T>
  concept Parsable =
    requires(const T& val) { std::istream{ val }; } ||
    requires(const T& val) { std::istringstream{ val }; };

  template <typename T>
  concept IterParsable =
    is_container<T> &&
    Parsable<typename T::value_type> &&
    requires(const T& val) {
      std::begin(val);
      std::end(val);
    };

  template <typename C, typename T1, typename T2>
  concept Chainable = CanStore<C, T2> && (Parsable<T1> || IterParsable<T1>);

  template <typename C, typename T1, typename T2>
  concept NotChainable = !Chainable<C, T1, T2>;

  template <typename T1, typename T2>
  using ParserChainPair = std::tuple<Ref<Parser<T1>>, Ref<Parser<T2>>>;

  template <typename C, typename T1, typename T2>
    requires Chainable<C, T1, T2>
  struct ParseInto : Parser<C> {
    const ParserChainPair<T1, T2> parsers;

    ParseInto(const Ref<Parser<T1>>& parser1, const Ref<Parser<T2>>& parser2)
        : parsers({ parser1, parser2 }) {}

    ParseInto(const ParserChainPair<T1, T2>& parsers)
        : parsers(parsers) {}

    C operator()(std::istream& stream) const override {
      auto& [parsers1, parser2] = parsers;

      T1 val = (*parsers1)(stream);
      if (stream.fail()) {
        return C{};
      }

      C result{};
      T2 val2{};
      if constexpr (Parsable<T1>) {
        std::istringstream stream2(val);
        val2 = (*parser2)(stream2);

        if (stream.fail()) {
          return C{};
        }
      } else if constexpr (IterParsable<T1>) {
        for (const auto& data : val) {
          std::istringstream stream2(data);
          val2 = (*parser2)(stream2);

          if (stream2.fail()) {
            return C{};
          }
          result.insert(result.end(), val2);
        }
      } else {
        static_assert(false, "Invalid type");
      }

      return result;
    }
  };

  template <typename C, typename T>
    requires Parsable<T> && std::same_as<T, typename C::value_type>
  struct ParseAndCollect : Parser<C> {
    const Ref<Parser<T>> parser1;
    const Ref<Parser<C>> parser2;

    ParseAndCollect(const Ref<Parser<T>>& parser1, const Ref<Parser<C>>& parser2)
        : parser1(parser1), parser2(parser2) {}

    C operator()(std::istream& stream) const override {
      T val = (*parser1)(stream);
      if (stream.fail()) {
        return C{};
      }

      if (val == T{}) {
        stream.setstate(std::ios_base::failbit);
        return C{};
      }

      std::istringstream str2(val);
      C result = (*parser2)(str2);
      if (str2.fail()) {
        return C{};
      }

      return result;
    }
  };

  template <typename C, typename T1, typename T2>
    requires Chainable<C, T1, T2>
  Ref<Parser<C>> Into(const Ref<Parser<T1>>& parser, const Ref<Parser<T2>>& into) {
    return NewRef<ParseInto<C, T1, T2>>(parser, into);
  }

  template <typename C, typename T1, typename T2>
    requires Chainable<C, T1, T2>
  Ref<Parser<C>> Into(const ParserChainPair<T1, T2>& parsers) {
    return NewRef<ParseInto<C, T1, T2>>(parsers);
  }

  template <typename C, typename T>
    requires Parsable<T> && std::same_as<T, typename C::value_type>
  Ref<Parser<C>> CollectInto(const Ref<Parser<T>>& parser, const Ref<Parser<C>>& into) {
    return NewRef<ParseAndCollect<C, T>>(parser, into);
  }

  template <typename T>
  struct MaybeParser : Parser<std::optional<T>> {
    Ref<Parser<T>> parser = nullptr;

    MaybeParser(const Ref<Parser<T>>& parser)
        : parser(parser) {}

    std::optional<T> operator()(std::istream& stream) const override {
      try {
        std::streamoff start_off = tellg(stream, std::ios::beg);

        T val = (*parser)(stream);
        if (!stream.fail()) {
          return val;
        }

        if (stream.eof()) {
          return std::nullopt;
        }

        stream.clear();

        std::streamoff curr_off = tellg(stream, std::ios::beg);
        update_stream(stream);
        if (start_off != curr_off) {
          stream.seekg(start_off);
        }

        return std::nullopt;
      } catch (...) {
        return std::nullopt;
      }
    }
  };

  template <typename T>
  Ref<Parser<std::optional<T>>> Maybe(Ref<Parser<T>> parser) {
    return NewRef<MaybeParser<T>>(parser);
  }

}  // namespace other

#endif  // !OTHER_ENGINE_PARSER_HPP

/**
 * \file parser_combinator_tests.cpp
 **/
#include <ranges>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "core/errors.hpp"

#include "parsing/parser_combinators.hpp"

#include "oetest.hpp"

using namespace other;

class ParserCombinatorTests : public OtherTest {
 protected:
};

TEST_F(ParserCombinatorTests, simple_character_parsers) {
  Ref<Parser<char>> parser = AnyMatcher();
  Ref<Parser<char>> parser2 = NoneMatcher();
  Ref<Parser<char>> parser3 = GroupMatcher("abc");

  std::istringstream stream("abc");
  char c = '\0';

  c = (*parser)(stream);
  EXPECT_EQ(c, 'a');
  c = (*parser)(stream);
  EXPECT_EQ(c, 'b');
  c = (*parser)(stream);
  EXPECT_EQ(c, 'c');
  /// match any matches EOF as well
  c = (*parser)(stream);
  EXPECT_EQ(c, EOF);

  stream = std::istringstream("abc");
  ASSERT_NO_THROW(c = (*parser2)(stream));
  EXPECT_EQ(c, '\0');
  ASSERT_NO_THROW(c = (*parser2)(stream));
  EXPECT_EQ(c, '\0');
  ASSERT_NO_THROW(c = (*parser2)(stream));
  EXPECT_EQ(c, '\0');
  ASSERT_NO_THROW(c = (*parser2)(stream));
  EXPECT_EQ(c, '\0');

  stream = std::istringstream("abc");
  ASSERT_NO_THROW(c = (*parser3)(stream));
  EXPECT_EQ(c, 'a');
  ASSERT_NO_THROW(c = (*parser3)(stream));
  EXPECT_EQ(c, 'b');
  ASSERT_NO_THROW(c = (*parser3)(stream));
  EXPECT_EQ(c, 'c');

  Ref<Parser<char>> parser4 = GroupMatcher("({[");
  stream = std::istringstream("({[");

  ASSERT_NO_THROW(c = (*parser4)(stream));
  EXPECT_EQ(c, '(');
  ASSERT_NO_THROW(c = (*parser4)(stream));
  EXPECT_EQ(c, '{');
  ASSERT_NO_THROW(c = (*parser4)(stream));
  EXPECT_EQ(c, '[');

  Ref<Parser<char>> parser5 = GroupMatcher("]})");
  stream = std::istringstream("]})");

  ASSERT_NO_THROW(c = (*parser5)(stream));
  EXPECT_EQ(c, ']');
  ASSERT_NO_THROW(c = (*parser5)(stream));
  EXPECT_EQ(c, '}');
  ASSERT_NO_THROW(c = (*parser5)(stream));
  EXPECT_EQ(c, ')');
}

TEST_F(ParserCombinatorTests, parse_or) {
  Ref<Parser<char>> parser = Char('a') | Char('b');

  std::istringstream stream("a");
  char c = '\0';

  ASSERT_NO_THROW(c = (*parser)(stream));
  EXPECT_EQ(c, 'a');

  stream = std::istringstream("b");
  ASSERT_NO_THROW(c = (*parser)(stream));
  EXPECT_EQ(c, 'b');

  stream = std::istringstream("c");
  ASSERT_THROW(c = (*parser)(stream), other::ParsingError);

  stream = std::istringstream("d");
  ASSERT_THROW(c = (*parser)(stream), other::ParsingError);

  Ref<Parser<std::string>> parser2 = (Char('a') + Char('b')) | Str(Char('c'));

  std::string str;

  stream = std::istringstream("ab");
  ASSERT_NO_THROW(str = (*parser2)(stream));
  EXPECT_EQ(str, "ab");

  stream = std::istringstream("c");
  ASSERT_NO_THROW(str = (*parser2)(stream));
  EXPECT_EQ(str, "c");

  stream = std::istringstream("d");
  ASSERT_THROW(str = (*parser2)(stream), other::ParsingError);
}

TEST_F(ParserCombinatorTests, grammar_building) {
  /**
   * toy grammer
   *  G :
   *      <S> -> '<' <E> '>'
   *      <E> -> <A> <B> | <C>
   *      <A> -> 'a'
   *      <B> -> 'b'
   *      <C> -> 'c' | 'd'
   **/

  /// in reverse order from above
  Ref<Parser<char>> c_parser = Char('c') | Char('d');
  Ref<Parser<char>> b_parser = Char('b');
  Ref<Parser<char>> a_parser = Char('a');
  Ref<Parser<std::string>> e_parser = (a_parser + b_parser) | c_parser;
  auto s_parser = ParseMultiple(Char('<'), e_parser, Char('>'));

  std::tuple<char, std::string, char> result;

  std::istringstream stream("<ab>");
  ASSERT_NO_THROW(result = (*s_parser)(stream));
  EXPECT_EQ(std::get<0>(result), '<');
  EXPECT_EQ(std::get<1>(result), "ab");
  EXPECT_EQ(std::get<2>(result), '>');

  std::istringstream stream2("<c>");
  ASSERT_NO_THROW(result = (*s_parser)(stream2));
  EXPECT_EQ(std::get<0>(result), '<');
  EXPECT_EQ(std::get<1>(result), "c");
  EXPECT_EQ(std::get<2>(result), '>');

  std::istringstream stream3("<d>");
  ASSERT_NO_THROW(result = (*s_parser)(stream3));
  EXPECT_EQ(std::get<0>(result), '<');
  EXPECT_EQ(std::get<1>(result), "d");
  EXPECT_EQ(std::get<2>(result), '>');
}

TEST_F(ParserCombinatorTests, contatenate_parsers) {
  Ref<Parser<std::string>> parser = Str(Char('a'));
  Ref<Parser<std::string>> parser2 = Char('a') + Char('b');
  Ref<Parser<std::string>> parser3 = Char('a') + Char('b') + Char('c');

  std::istringstream stream("aaa");
  std::string str;

  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "a");
  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "a");
  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "a");

  stream = std::istringstream("ab");
  ASSERT_NO_THROW(str = (*parser2)(stream));
  EXPECT_EQ(str, "ab");

  stream = std::istringstream("abc");
  ASSERT_NO_THROW(str = (*parser3)(stream));
  EXPECT_EQ(str, "abc");
}

TEST_F(ParserCombinatorTests, string_parser) {
  Ref<Parser<std::string>> parser = MatchString("abc");

  std::istringstream stream("abc");
  std::string str;

  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "abc");

  stream = std::istringstream("abcd");
  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "abc");
  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "");
}

TEST_F(ParserCombinatorTests, simple_skip_parsers) {
  Ref<Parser<void>> parser = Skip('a');
  Ref<Parser<void>> parser2 = Skip("abc");

  std::istringstream stream("a");
  ASSERT_NO_THROW((*parser)(stream));

  stream = std::istringstream("abc");
  ASSERT_NO_THROW((*parser)(stream));

  ASSERT_NO_THROW((*parser)(stream));
  stream.clear();
  ASSERT_EQ(char(stream.get()), 'b');

  ASSERT_NO_THROW((*parser)(stream));
  stream.clear();
  ASSERT_EQ(char(stream.get()), 'c');

  stream = std::istringstream("abc");
  ASSERT_NO_THROW((*parser2)(stream));
  ASSERT_THROW((*parser2)(stream), other::ParsingError);
}

TEST_F(ParserCombinatorTests, simple_map_parsers) {
  auto parser = Char('a') | [](char c) -> int {
    return 1;
  };

  Ref<Parser<int>> parser2 = parser | [](int i) -> int {
    return i + 1;
  };

  std::istringstream stream("a");
  int i = 0;

  ASSERT_NO_THROW(i = (*parser)(stream));
  EXPECT_EQ(i, 1);

  stream = std::istringstream("a");
  ASSERT_NO_THROW(i = (*parser2)(stream));
  EXPECT_EQ(i, 2);

  Ref<Parser<std::string>> parser3 = GroupMatcher("abc") | [](char c) -> std::string {
    auto str = std::string(2, c);
    std::cout << "mapped to : " << str << std::endl;
    return str;
  };

  std::istringstream stream2("abc");
  std::string str;

  ASSERT_NO_THROW(str = (*parser3)(stream2));
  EXPECT_EQ(str, "aa");
  ASSERT_NO_THROW(str = (*parser3)(stream2));
  EXPECT_EQ(str, "bb");
  ASSERT_NO_THROW(str = (*parser3)(stream2));
  EXPECT_EQ(str, "cc");
}

TEST_F(ParserCombinatorTests, whitespace_trimmer) {
  Ref<Parser<std::string>> parser = SkipWhitespaceThenMatch("abc");

  std::istringstream stream("abc");
  std::string str;

  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "abc");

  stream = std::istringstream(" abc");
  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "abc");

  stream = std::istringstream("     \n   abc");
  ASSERT_NO_THROW(str = (*parser)(stream));
}

TEST_F(ParserCombinatorTests, simple_sequence_parsers) {
  Ref<Parser<std::string>> parser = Seq(Seq(Char('a'), Str(Char('b'))), Str(Char('c')));

  std::istringstream stream("abc");
  std::string str;

  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "abc");

  stream = std::istringstream("abcd");
  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "abc");

  Ref<Parser<std::string>> parser2 = MatchAlpha() + ParseUntil(" \n\t\r");

  stream = std::istringstream("abc");
  ASSERT_NO_THROW(str = (*parser2)(stream));
  EXPECT_EQ(str, "abc");

  stream = std::istringstream("abc ");
  ASSERT_NO_THROW(str = (*parser2)(stream));
  EXPECT_EQ(str, "abc");

  stream = std::istringstream("abc\n");
  ASSERT_NO_THROW(str = (*parser2)(stream));
  EXPECT_EQ(str, "abc");

  stream = std::istringstream("abc\t");
  ASSERT_NO_THROW(str = (*parser2)(stream));
  EXPECT_EQ(str, "abc");
}

TEST_F(ParserCombinatorTests, fallback_parser) {
  Ref<Parser<char>> parser = Char('a') | (Char('b') | Char('c'));

  std::istringstream stream("a");
  std::string str;

  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "a");

  stream = std::istringstream("b");
  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "b");

  stream = std::istringstream("c");
  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "c");

  stream = std::istringstream("d");
  ASSERT_THROW(str = (*parser)(stream), other::ParsingError);
}

TEST_F(ParserCombinatorTests, parse_string_trim_whitespace) {
  Ref<Parser<std::string>> parser = MatchAndTrim("abc");

  std::istringstream stream("abc");
  std::string str;

  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "abc");

  stream = std::istringstream(" abc");
  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "abc");

  stream = std::istringstream("abc ");
  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "abc");

  stream = std::istringstream(" abc ");
  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "abc");
}

TEST_F(ParserCombinatorTests, simple_parse_many) {
  Ref<Parser<std::string>> parser = Many<std::string>(Char('a'));

  std::istringstream stream("aaa");
  std::string str;

  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "aaa");

  stream = std::istringstream("aaaa");
  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "aaaa");

  stream = std::istringstream("aaaaa");
  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "aaaaa");

  stream = std::istringstream("aaaba");
  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "aaa");

  Ref<Parser<std::string>> parser2 = Many<std::string>(Char('a')) + Char('b');

  stream = std::istringstream("ab");
  ASSERT_NO_THROW(str = (*parser2)(stream));
  EXPECT_EQ(str, "ab");

  stream = std::istringstream("aab");
  ASSERT_NO_THROW(str = (*parser2)(stream));
  EXPECT_EQ(str, "aab");

  stream = std::istringstream("aaab");
  ASSERT_NO_THROW(str = (*parser2)(stream));
  EXPECT_EQ(str, "aaab");

  stream = std::istringstream("aaaab");
  ASSERT_NO_THROW(str = (*parser2)(stream));
  EXPECT_EQ(str, "aaaab");
}

TEST_F(ParserCombinatorTests, parse_ini_key_value) {
  Ref<Parser<std::vector<std::string>>> parser = SplitStringOn('=');

  std::istringstream stream("key=value");
  std::vector<std::string> pair;

  ASSERT_NO_THROW(pair = (*parser)(stream));
  ASSERT_EQ(pair.size(), 2);
  EXPECT_EQ(pair[0], "key");
  EXPECT_EQ(pair[1], "value");

  stream = std::istringstream("key = value");
  ASSERT_NO_THROW(pair = (*parser)(stream));
  ASSERT_EQ(pair.size(), 2);
  EXPECT_EQ(pair[0], "key");
  EXPECT_EQ(pair[1], "value");
}

TEST_F(ParserCombinatorTests, parse_simple_ini_section) {
  Ref<Parser<std::vector<std::string>>> key_value_parser = SplitStringOn('\n');
  Ref<Parser<std::vector<std::string>>> split_key_value_parser = SplitStringOn('=');

  // clang-format off
  Ref<Parser<std::vector<std::vector<std::string>>>> section_parser =
    key_value_parser |
    [&](const std::vector<std::string>& data) -> std::vector<std::vector<std::string>> {
      std::vector<std::vector<std::string>> section_data;
      for (auto& line : data) {
        std::istringstream line_stream(line);
        std::vector<std::string> data;
        data = (*split_key_value_parser)(line_stream);
        section_data.push_back(data);
      }
      return section_data;
    };
  // clang-format on

  std::istringstream stream("key1=value1\nkey2=value2\nkey3=value3");
  std::istringstream stream2("key1=value1\nkey2=value2\nkey3=value3\n");
  std::vector<std::string> section;

  ASSERT_NO_THROW(section = (*key_value_parser)(stream));
  ASSERT_EQ(section.size(), 3);

  std::vector<std::vector<std::string>> section_data;
  for (auto& line : section) {
    std::istringstream line_stream(line);
    std::vector<std::string> data;
    ASSERT_NO_THROW(data = (*split_key_value_parser)(line_stream));
    ASSERT_EQ(data.size(), 2);
    section_data.push_back(data);
  }

  ASSERT_EQ(section_data[0].size(), 2);
  EXPECT_EQ(section_data[0][0], "key1");
  EXPECT_EQ(section_data[0][1], "value1");

  ASSERT_EQ(section_data[1].size(), 2);
  EXPECT_EQ(section_data[1][0], "key2");
  EXPECT_EQ(section_data[1][1], "value2");

  ASSERT_EQ(section_data[2].size(), 2);
  EXPECT_EQ(section_data[2][0], "key3");
  EXPECT_EQ(section_data[2][1], "value3");

  ASSERT_NO_THROW(section_data = (*section_parser)(stream2));
  section_data =
    section_data |
    std::views::filter([](const std::vector<std::string>& data) { return data.size() == 2; }) |
    std::ranges::to<std::vector<std::vector<std::string>>>();

  ASSERT_EQ(section_data.size(), 3);
  for (size_t i = 0; i < section_data.size(); ++i) {
    ASSERT_EQ(section_data[i].size(), 2);
    EXPECT_EQ(section_data[i][0], "key" + std::to_string(i + 1));
    EXPECT_EQ(section_data[i][1], "value" + std::to_string(i + 1));
  }
}

TEST_F(ParserCombinatorTests, simple_for_each) {
  Ref<Parser<std::vector<std::vector<std::string>>>> parser = ForEach<std::vector<std::vector<std::string>>>(SplitStringOn('\n'), SplitStringOn('='));
  std::istringstream stream("key1=value1\nkey2=value2\nkey3=value3");

  std::vector<std::vector<std::string>> section_data;
  ASSERT_NO_THROW(section_data = (*parser)(stream));
  ASSERT_EQ(section_data.size(), 3);

  for (size_t i = 0; i < section_data.size(); ++i) {
    ASSERT_EQ(section_data[i].size(), 2);
    EXPECT_EQ(section_data[i][0], "key" + std::to_string(i + 1));
    EXPECT_EQ(section_data[i][1], "value" + std::to_string(i + 1));
  }
}

TEST_F(ParserCombinatorTests, parse_ini_piece_by_piece) {
  std::istringstream stream("[section1]\nkey1=value1\nkey2=value2\nkey3=value3");

  Ref<Parser<std::vector<std::string>>> splitter = SplitStringOn('\n');

  std::vector<std::string> split_string;
  ASSERT_NO_THROW(split_string = (*splitter)(stream));
  ASSERT_EQ(split_string.size(), 4);

  EXPECT_EQ(split_string[0], "[section1]");
  EXPECT_EQ(split_string[1], "key1=value1");
  EXPECT_EQ(split_string[2], "key2=value2");
  EXPECT_EQ(split_string[3], "key3=value3");

  Ref<Parser<std::string>> header_parser = MatchIdentifierAndStripParens();

  std::istringstream header_stream(split_string[0]);
  std::string section_header;
  ASSERT_NO_THROW(section_header = (*header_parser)(header_stream));
  EXPECT_EQ(section_header, "section1");

  /// remove the section header
  split_string.erase(split_string.begin());

  Ref<Parser<std::vector<std::string>>> key_val_splitter = SplitStringOn('=');
  std::vector<std::vector<std::string>> section_data;

  for (auto& line : split_string) {
    std::istringstream line_stream(line);
    std::vector<std::string> data;
    ASSERT_NO_THROW(data = (*key_val_splitter)(line_stream));
    ASSERT_EQ(data.size(), 2);
    section_data.push_back(data);
  }

  ASSERT_EQ(section_data.size(), 3);

  ASSERT_EQ(section_data[0].size(), 2);
  EXPECT_EQ(section_data[0][0], "key1");
  EXPECT_EQ(section_data[0][1], "value1");

  ASSERT_EQ(section_data[1].size(), 2);
  EXPECT_EQ(section_data[1][0], "key2");
  EXPECT_EQ(section_data[1][1], "value2");

  ASSERT_EQ(section_data[2].size(), 2);
  EXPECT_EQ(section_data[2][0], "key3");
  EXPECT_EQ(section_data[2][1], "value3");
}

TEST_F(ParserCombinatorTests, fallback_parsers) {
  Ref<Parser<std::string>> parser = MatchString("abc") | GroupMatcher("bc");  // abc or (b or c)

  std::istringstream stream("abc");
  std::stringstream stream2("abcbasdfkasdfjker");
  std::istringstream stream3("b");
  std::istringstream stream4("c");
  std::istringstream stream5("d");
  std::string str;

  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "abc");

  ASSERT_NO_THROW(str = (*parser)(stream2));
  EXPECT_EQ(str, "abc");

  ASSERT_NO_THROW(str = (*parser)(stream3));
  EXPECT_EQ(str, "b");

  ASSERT_NO_THROW(str = (*parser)(stream4));
  EXPECT_EQ(str, "c");

  ASSERT_THROW(str = (*parser)(stream5), other::ParsingError);

  Ref<Parser<std::string>> parser2 = Many<std::string>(MatchAlnum() | Char('_'));

  stream = std::istringstream("abc");
  ASSERT_NO_THROW(str = (*parser2)(stream));
  EXPECT_EQ(str, "abc");

  stream2 = std::stringstream("abc123");
  ASSERT_NO_THROW(str = (*parser2)(stream2));
  EXPECT_EQ(str, "abc123");

  stream3 = std::istringstream("_");
  ASSERT_NO_THROW(str = (*parser2)(stream3));
  EXPECT_EQ(str, "_");

  stream4 = std::istringstream("abc123_123");
  ASSERT_NO_THROW(str = (*parser2)(stream4));
  EXPECT_EQ(str, "abc123_123");

  stream5 = std::istringstream("1_");
  ASSERT_NO_THROW(str = (*parser2)(stream5));
  EXPECT_EQ(str, "1_");

  std::istringstream stream6 = std::istringstream("_1");
  ASSERT_NO_THROW(str = (*parser2)(stream6));
  EXPECT_EQ(str, "_1");

  std::istringstream stream7 = std::istringstream("2_2");
  ASSERT_NO_THROW(str = (*parser2)(stream7));
  EXPECT_EQ(str, "2_2");

  std::istringstream stream8 = std::istringstream("2_2_2");
  ASSERT_NO_THROW(str = (*parser2)(stream8));
  EXPECT_EQ(str, "2_2_2");

  std::istringstream stream9 = std::istringstream("xyz_123_abc");
  ASSERT_NO_THROW(str = (*parser2)(stream9));
  EXPECT_EQ(str, "xyz_123_abc");

  std::istringstream stream10 = std::istringstream("xyz123abc_");
  ASSERT_NO_THROW(str = (*parser2)(stream10));
  EXPECT_EQ(str, "xyz123abc_");

  std::istringstream stream11 = std::istringstream("abc123_");
  ASSERT_NO_THROW(str = (*parser2)(stream11));
  EXPECT_EQ(str, "abc123_");

  std::istringstream stream12 = std::istringstream("abc123_1");
  ASSERT_NO_THROW(str = (*parser2)(stream12));
  EXPECT_EQ(str, "abc123_1");
}

TEST_F(ParserCombinatorTests, simple_multiparser) {
  Ref<Parser<std::tuple<char, char>>> parser = ParseMultiple(Char('a'), Char('b'));

  std::istringstream stream("ab");
  std::pair<char, char> res;
  ASSERT_NO_THROW(res = (*parser)(stream));

  auto [r1, r2] = res;
  EXPECT_EQ(r1, 'a');
  EXPECT_EQ(r2, 'b');

  auto parser2 = ParseMultiple(Char('a'), Char('b'), Char('c'));

  stream = std::istringstream("abc");
  std::tuple<char, char, char> res2;
  ASSERT_NO_THROW(res2 = (*parser2)(stream));

  auto [r3, r4, r5] = res2;
  EXPECT_EQ(r3, 'a');
  EXPECT_EQ(r4, 'b');
  EXPECT_EQ(r5, 'c');

  auto parser3 = ParseMultiple(MatchString("abc"), MatchString("def"), MatchString("ghi"));

  stream = std::istringstream("abcdefghi");
  std::tuple<std::string, std::string, std::string> res3;
  ASSERT_NO_THROW(res3 = (*parser3)(stream));

  auto [r6, r7, r8] = res3;
  EXPECT_EQ(r6, "abc");
  EXPECT_EQ(r7, "def");
  EXPECT_EQ(r8, "ghi");

  auto parser4 = ParseMultiple(MatchString("abc"), Char('d'));

  stream = std::istringstream("abcd");
  std::tuple<std::string, char> res4;
  ASSERT_NO_THROW(res4 = (*parser4)(stream));

  auto [r9, r10] = res4;
  EXPECT_EQ(r9, "abc");
  EXPECT_EQ(r10, 'd');
}

TEST_F(ParserCombinatorTests, parse_into) {
  Ref<Parser<std::vector<std::string>>> line_splitter = SplitStringOn('\n');
  Ref<Parser<std::vector<std::string>>> key_val_splitter = SplitStringOn('=');

  auto parser = Into<std::vector<std::vector<std::string>>>(line_splitter, key_val_splitter);

  std::istringstream stream("key1=value1\nkey2=value2\nkey3=value3");

  std::vector<std::vector<std::string>> section_data;
  ASSERT_NO_THROW(section_data = (*parser)(stream));
  ASSERT_EQ(section_data.size(), 3);

  for (size_t i = 0; i < section_data.size(); ++i) {
    ASSERT_EQ(section_data[i].size(), 2);
    EXPECT_EQ(section_data[i][0], "key" + std::to_string(i + 1));
    EXPECT_EQ(section_data[i][1], "value" + std::to_string(i + 1));
  }

  Ref<Parser<std::vector<std::string>>> comma_splitter = SplitStringOn(',');
  Ref<Parser<std::vector<std::string>>> string_matcher = SplitStringOn('l');
  auto parser2 = Into<std::vector<std::vector<std::string>>>(comma_splitter, string_matcher);

  stream = std::istringstream("kasdlaksdfj,asdflaskdfjasdf,asdlfkajsdflkasd,asdlfkjasdlfkasdf,asdlfkjasdlfkasdlfkjasdflkasdf\n");
  ASSERT_NO_THROW(section_data = (*parser2)(stream));

  ASSERT_EQ(section_data.size(), 5);
  for (size_t i = 0; i < section_data.size(); ++i) {
    ASSERT_EQ(section_data[0].size(), 2);
  }

  EXPECT_EQ(section_data[0][0], "kasd");
  EXPECT_EQ(section_data[0][1], "aksdfj");

  EXPECT_EQ(section_data[1][0], "asdf");
  EXPECT_EQ(section_data[1][1], "askdfjasdf");

  EXPECT_EQ(section_data[2][0], "asd");
  EXPECT_EQ(section_data[2][1], "fkajsdf");

  EXPECT_EQ(section_data[3][0], "asd");
  EXPECT_EQ(section_data[3][1], "fkjasd");

  EXPECT_EQ(section_data[4][0], "asd");
  EXPECT_EQ(section_data[4][1], "fkjasd");
}

TEST_F(ParserCombinatorTests, complicated_grammar) {
  /**
   * G =
   *    <S> ::= '[' <identifier> ']' <data-block>
   *    <data-block> ::= <data>*  | '{' <data-block>* '}'
   *    <data> ::= <identifier> '=' <value>
   *    <identifier> ::= [a-zA-Z_][a-zA-Z0-9_]*
   *    <value> ::= <identifier> | 'true' | 'false' | <digit>
   *    <digit> ::= [0-9]
   **/

  Ref<Parser<std::string>> header_parser = MatchIdentifierAndStripParens();
  auto skip_newline = Skip('\n');
  // clang-format off
  auto line_splitter = Into<std::vector<std::vector<std::string>>>(SplitStringOn('\n'), SplitStringOn('=')) |
    [](const std::vector<std::vector<std::string>>& data) -> std::vector<std::vector<std::string>> {
      return data |
        std::views::filter([](const std::vector<std::string>& data) { return !data.empty(); }) |
        std::ranges::to<std::vector<std::vector<std::string>>>();
    };
  // clang-format on

  std::istringstream stream("[section1]\nkey1=value1\nkey2=value2\nkey3=value3\nkey4=false\nkey_5=1");

  std::string section;
  ASSERT_NO_THROW(section = (*header_parser)(stream));
  EXPECT_EQ(section, "section1");

  (*skip_newline)(stream);
  ASSERT_FALSE(stream.fail());
  ASSERT_FALSE(stream.eof());
  ASSERT_EQ(char(stream.peek()), 'k');

  std::vector<std::vector<std::string>> data;
  ASSERT_NO_THROW(data = (*line_splitter)(stream));
  ASSERT_EQ(data.size(), 5u);

  EXPECT_EQ(data[0][0], "key1");
  EXPECT_EQ(data[0][1], "value1");

  EXPECT_EQ(data[1][0], "key2");
  EXPECT_EQ(data[1][1], "value2");

  EXPECT_EQ(data[2][0], "key3");
  EXPECT_EQ(data[2][1], "value3");

  EXPECT_EQ(data[3][0], "key4");
  EXPECT_EQ(data[3][1], "false");

  EXPECT_EQ(data[4][0], "key_5");
  EXPECT_EQ(data[4][1], "1");

  auto parse_identifier = MatchIdentifier();
  auto skip_equals = SkipSpaces() >> Skip('=') >> SkipSpaces();

  auto parse_value_list = ParseMultiple(Char('{'), MatchAnyStringWithout("}") | &TrimBeginningAndEnd, Char('}'));
  auto parse_value = parse_identifier | MatchString("true") | MatchString("false") | MatchDigit();

  stream = std::istringstream("[section2]\nkey1=value1\nkey2={key3=value3\nkey4=false\nkey_5=1\n}");

  ASSERT_NO_THROW(section = (*header_parser)(stream));
  EXPECT_EQ(section, "section2");

  (*skip_newline)(stream);
  ASSERT_FALSE(stream.fail());
  ASSERT_FALSE(stream.eof());
  ASSERT_EQ(char(stream.peek()), 'k');

  std::string key;
  std::string value;
  std::tuple<char, std::string, char> value_list;

  ASSERT_NO_THROW(key = (*parse_identifier)(stream));
  EXPECT_EQ(key, "key1");

  (*skip_equals)(stream);
  ASSERT_FALSE(stream.fail());
  ASSERT_FALSE(stream.eof());
  ASSERT_EQ(char(stream.peek()), 'v');

  ASSERT_NO_THROW(value = (*parse_value)(stream));
  EXPECT_EQ(value, "value1");

  (*skip_newline)(stream);
  ASSERT_FALSE(stream.fail());
  ASSERT_FALSE(stream.eof());

  ASSERT_NO_THROW(key = (*parse_identifier)(stream));
  EXPECT_EQ(key, "key2");

  (*skip_equals)(stream);
  ASSERT_FALSE(stream.fail());
  ASSERT_FALSE(stream.eof());
  ASSERT_EQ(char(stream.peek()), '{');

  ASSERT_NO_THROW(value_list = (*parse_value_list)(stream));
  auto [c, value_list_str, c2] = value_list;

  EXPECT_EQ(c, '{');
  EXPECT_EQ(value_list_str, "key3=value3\nkey4=false\nkey_5=1");
  EXPECT_EQ(c2, '}');

  stream = std::istringstream(value_list_str);
  std::vector<std::vector<std::string>> value_data;
  ASSERT_NO_THROW(value_data = (*line_splitter)(stream));
  ASSERT_EQ(value_data.size(), 3u);

  EXPECT_EQ(value_data[0][0], "key3");
  EXPECT_EQ(value_data[0][1], "value3");

  EXPECT_EQ(value_data[1][0], "key4");
  EXPECT_EQ(value_data[1][1], "false");

  EXPECT_EQ(value_data[2][0], "key_5");
  EXPECT_EQ(value_data[2][1], "1");
}

TEST_F(ParserCombinatorTests , maybe_parser) {
  auto maybe_c = Maybe(Char('c'));

  Opt<char> empty;
  Opt<char> not_empty;

  std::istringstream str1("d");
  std::istringstream str2("c");

  ASSERT_NO_THROW(empty = (*maybe_c)(str1));
  ASSERT_EQ(char(str1.peek()), 'd');
  ASSERT_FALSE(empty.has_value());

  ASSERT_NO_THROW(not_empty = (*maybe_c)(str2));
  ASSERT_TRUE(not_empty.has_value());
  ASSERT_EQ(*not_empty, 'c');
}

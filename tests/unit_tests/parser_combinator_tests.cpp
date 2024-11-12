/**
 * \file parser_combinator_tests.cpp
 **/
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "core/errors.hpp"

#include "parsing/parser.hpp"

#include "oetest.hpp"

using namespace other;

class ParserCombinatorTests : public OtherTest {
 protected:
};

TEST_F(ParserCombinatorTests, simple_character_parsers) {
  Ref<Parser<char>> parser = AnyMatcher();
  Ref<Parser<char>> parser2 = NoneMatcher();
  Ref<Parser<char>> parser3 = GroupMatcher("abc");
  Ref<Parser<char>> parser4 = ExcludeGroup("abc");

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
  ASSERT_THROW(c = (*parser)(stream), other::ParsingError);

  stream = std::istringstream("abc");
  ASSERT_NO_THROW(c = (*parser2)(stream));
  EXPECT_EQ(c, '\0');
  ASSERT_THROW(c = (*parser2)(stream), other::ParsingError);

  stream.clear();
  stream.seekg(0, std::ios::beg);
  ASSERT_NO_THROW(c = (*parser3)(stream));
  EXPECT_EQ(c, 'a');
  ASSERT_NO_THROW(c = (*parser3)(stream));
  EXPECT_EQ(c, 'b');
  ASSERT_NO_THROW(c = (*parser3)(stream));
  EXPECT_EQ(c, 'c');
  ASSERT_NO_THROW(c = (*parser3)(stream));
  /// none of the characters in the set
  EXPECT_EQ(c, '\0');
  ASSERT_THROW(c = (*parser3)(stream), other::ParsingError);

  stream = std::istringstream("abc");
  ASSERT_NO_THROW(c = (*parser4)(stream));
  EXPECT_EQ(c, '\0');
  ASSERT_THROW(c = (*parser4)(stream), other::ParsingError);
}

TEST_F(ParserCombinatorTests, contatenate_parsers) {
  Ref<Parser<std::string>> parser = +Char('a');
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
  ASSERT_THROW(str = (*parser)(stream), other::ParsingError);
}

TEST_F(ParserCombinatorTests, simple_skip_parsers) {
  Ref<Parser<void>> parser = Skip('a');
  Ref<Parser<void>> parser2 = Skip("abc");

  std::istringstream stream("a");
  ASSERT_NO_THROW((*parser)(stream));
  ASSERT_NO_THROW((*parser)(stream));
  ASSERT_THROW((*parser)(stream), other::ParsingError);

  stream = std::istringstream("abc");
  ASSERT_NO_THROW((*parser)(stream));
  ASSERT_NO_THROW((*parser)(stream));
  ASSERT_THROW((*parser)(stream), other::ParsingError);

  stream = std::istringstream("abc");
  ASSERT_NO_THROW((*parser2)(stream));
  ASSERT_THROW((*parser2)(stream), other::ParsingError);
}

TEST_F(ParserCombinatorTests, simple_map_parsers) {
  // clang-format off
  auto parser = 
    Char('a') | 
    [](char c) -> int {
      return 1;
    };

  Ref<Parser<int>> parser2 = 
    parser |
    [](int i) -> int {
      return i + 1;
    };
  // clang-format on

  std::istringstream stream("a");
  int i = 0;

  ASSERT_NO_THROW(i = (*parser)(stream));
  EXPECT_EQ(i, 1);

  stream = std::istringstream("a");
  ASSERT_NO_THROW(i = (*parser2)(stream));
  EXPECT_EQ(i, 2);
}

TEST_F(ParserCombinatorTests, simple_sequence_parsers) {
  Ref<Parser<std::string>> parser = Seq(Seq(Char('a'), +Char('b')), +Char('c'));

  std::istringstream stream("abc");
  std::string str;

  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "abc");

  stream = std::istringstream("abcd");
  ASSERT_NO_THROW(str = (*parser)(stream));
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

TEST_F(ParserCombinatorTests, parse_ini_section_header) {
  Ref<Parser<std::string>> parser = MatchIdentifierAndStripParens();

  std::istringstream stream("[section1]");
  std::string str;

  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "section1");
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
  std::string str;

  ASSERT_NO_THROW(str = (*parser)(stream));
  EXPECT_EQ(str, "abc");

  ASSERT_NO_THROW(str = (*parser)(stream2));
  EXPECT_EQ(str, "abc");

  /// doesnt match cause first char is a
  ASSERT_THROW(str = (*parser)(stream3), other::ParsingError);
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

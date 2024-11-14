/**
 * \file ini_parser_tests.cpp
 **/
#include <gtest/gtest.h>

#include "parsing/ini_parser.hpp"
#include "parsing/parser_combinators.hpp"

#include "oetest.hpp"

using namespace other;

class IniParserTests : public other::OtherTest {
 protected:
};

TEST_F(IniParserTests, simple_grammar) {
  /**
   * Grammar:
   *    <ini-file> ::= <section>*
   *    <section> ::= <section-header> <key-value>*
   *    <section-header> ::= '[' <identifier> ']'
   *    <key-value> ::= <identifier> '=' <identifier>
   *    <identifier> ::= [a-zA-Z_][a-zA-Z0-9_]*
   **/

  auto section_parser = ParseMultiple(MatchIdentifierAndStripParens(), ForEach<std::vector<std::vector<std::string>>>(SplitStringOn('\n'), SplitStringOn('=')));

  std::string ini_data =
    R"([section1]
  key1=value1
  key2=value2
  key3=value3)";

  std::istringstream stream(ini_data);

  std::tuple<std::string, std::vector<std::vector<std::string>>> section;
  ASSERT_NO_THROW(section = (*section_parser)(stream));
  auto [header, kv_pairs] = section;

  EXPECT_EQ(header, "section1");
  ASSERT_EQ(kv_pairs.size(), 3);

  for (size_t i = 0; i < kv_pairs.size(); ++i) {
    ASSERT_EQ(kv_pairs[i].size(), 2);
    EXPECT_EQ(kv_pairs[i][0], "key" + std::to_string(i + 1));
    EXPECT_EQ(kv_pairs[i][1], "value" + std::to_string(i + 1));
  }
}

TEST_F(IniParserTests, ini_combinator) {
  std::string ini_data =
    R"([section1]
  key1 = value1
  key2 = value2
  key3 = value3
  key4 = { value1, value2, value3 }
  )";
  // # comment
  // [section2]
  // key1 = value1
  // key2 = value2
  // key3 = value3
  std::istringstream stream(ini_data);

  using section_t = std::pair<std::string, std::pair<std::string, std::string>>;
  std::vector<section_t> sections;

  Ref<Parser<section_t>> section_parser = ParseMultiple(MatchIdentifierAndStripParens(), ForEach<std::vector<std::vector<std::string>>>(SplitStringOn('\n'), SplitStringOn('=')));

  Ref<Parser<std::string>> header_parser = MatchIdentifierAndStripParens();
  Ref<Parser<std::string>> section_data_parser = ParseUntil('[');

  Ref<Parser<std::vector<std::string>>> line_sep_parser = SplitStringOn('\n') | [](const std::vector<std::string>& vec) -> std::vector<std::string> {
    return vec |
      std::views::filter([](const std::string& str) { return !str.empty(); }) |
      std::ranges::to<std::vector<std::string>>();
  };
  Ref<Parser<std::vector<std::string>>> key_val_parser = SplitStringOn('=');

  Ref<Parser<void>> skip_comment = SkipUntil('\n');
  Ref<Parser<void>> skip_newline = Skip('\n');
  Ref<Parser<void>> skip_any = SkipAny();

  try {
    while (!stream.eof()) {
      switch (stream.peek()) {
        case '#':
          (*skip_comment)(stream);
          continue;

        case '[': {
          std::string title = (*header_parser)(stream);
          std::string keys = (*section_data_parser)(stream);

          std::istringstream keys_stream(keys);
          std::vector<std::string> key_vals = (*line_sep_parser)(keys_stream);
          ASSERT_EQ(key_vals.size(), 4);

          for (auto& key_val : key_vals) {
            std::istringstream key_val_stream(key_val);
            std::vector<std::string> kv = (*key_val_parser)(key_val_stream);
            ASSERT_EQ(kv.size(), 2);

            sections.push_back({ title, { kv[0], kv[1] } });
          }

        } break;

        case '\n':
          (*skip_newline)(stream);
          break;

        case ' ':
        case '\t':
          (*skip_any)(stream);
          break;

        default:
          (*skip_any)(stream);
          break;
      }
    }
  } catch (const ParsingError& e) {
    FAIL();
  }

  // ASSERT_EQ(sections.size(), 1);
  for (auto& [title, kv] : sections) {
    std::cout << title << " " << kv.first << " = " << kv.second << std::endl;
  }

  // auto s1 = sections[0];
  // auto s2 = sections[1];

  // EXPECT_EQ(s1.first, "section1");
  // EXPECT_EQ(s2.first, "section2");

  // auto [k1, v1] = s1.second;
  // auto [k2, v2] = s2.second;

  // EXPECT_EQ(k1, "key1");
  // EXPECT_EQ(v1, "value1");

  // EXPECT_EQ(k2, "key1");
  // EXPECT_EQ(v2, "value1");
}

TEST_F(IniParserTests, empty_sections) {
  std::string ini_data = "[section1]";

  IniFileParser parser;
  ConfigTable config;
  ASSERT_NO_THROW(config = parser.Parse(ini_data));

  EXPECT_TRUE(config.SectionExists("SECTION1"));
  EXPECT_EQ(config.Get("SECTION1").size(), 0);

  /// with new line
  ini_data = "[section1]\n[section2]";
  ASSERT_NO_THROW(config = parser.Parse(ini_data));

  EXPECT_TRUE(config.SectionExists("SECTION1"));
  EXPECT_EQ(config.Get("SECTION1").size(), 0);

  EXPECT_TRUE(config.SectionExists("SECTION2"));
  EXPECT_EQ(config.Get("SECTION2").size(), 0);

  /// without new line
  ini_data = "[section1][section2]";
  ASSERT_NO_THROW(config = parser.Parse(ini_data));

  EXPECT_TRUE(config.SectionExists("SECTION1"));
  EXPECT_EQ(config.Get("SECTION1").size(), 0);

  EXPECT_TRUE(config.SectionExists("SECTION2"));
  EXPECT_EQ(config.Get("SECTION2").size(), 0);

  /// with comments
  ini_data = "[section1] # comment\n[section2] # comment";
  ASSERT_NO_THROW(config = parser.Parse(ini_data));

  EXPECT_TRUE(config.SectionExists("SECTION1"));
  EXPECT_EQ(config.Get("SECTION1").size(), 0);

  EXPECT_TRUE(config.SectionExists("SECTION2"));
  EXPECT_EQ(config.Get("SECTION2").size(), 0);

  /// with multiline comments
  ini_data = R"(
  [section1] # comment
  #[
    multiline comment
  #]
  [section2] # comment
  )";

  ASSERT_NO_THROW(config = parser.Parse(ini_data));

  EXPECT_TRUE(config.SectionExists("SECTION1"));
  EXPECT_EQ(config.Get("SECTION1").size(), 0);

  EXPECT_TRUE(config.SectionExists("SECTION2"));
  EXPECT_EQ(config.Get("SECTION2").size(), 0);
}

TEST_F(IniParserTests, simple_keys) {
  std::string ini_data = R"(
  [section1]
  key1 = value1
  key2 = value2
  key3 = value3
  [section2]
  key1 = { value1 }
  key2 = { value1, value2 }
  key3 = { value1, value2, value3 }
  )";

  IniFileParser parser;
  ConfigTable config;
  ASSERT_NO_THROW(config = parser.Parse(ini_data));

  ASSERT_TRUE(config.SectionExists("SECTION1"));
  EXPECT_EQ(config.Get("SECTION1").size(), 3);

  EXPECT_TRUE(config.KeyExists("SECTION1", "KEY1"));
  EXPECT_TRUE(config.KeyExists("SECTION1", "KEY2"));
  EXPECT_TRUE(config.KeyExists("SECTION1", "KEY3"));

  ASSERT_EQ(config.Get("SECTION1", "KEY1").size(), 1);
  ASSERT_EQ(config.Get("SECTION1", "KEY2").size(), 1);
  ASSERT_EQ(config.Get("SECTION1", "KEY3").size(), 1);

  EXPECT_EQ(config.Get("SECTION1", "KEY1")[0], "value1");
  EXPECT_EQ(config.Get("SECTION1", "KEY2")[0], "value2");
  EXPECT_EQ(config.Get("SECTION1", "KEY3")[0], "value3");

  ASSERT_TRUE(config.SectionExists("SECTION2"));
  EXPECT_EQ(config.Get("SECTION2").size(), 3);

  EXPECT_TRUE(config.KeyExists("SECTION2", "KEY1"));
  EXPECT_TRUE(config.KeyExists("SECTION2", "KEY2"));
  EXPECT_TRUE(config.KeyExists("SECTION2", "KEY3"));

  ASSERT_EQ(config.Get("SECTION2", "KEY1").size(), 1);
  ASSERT_EQ(config.Get("SECTION2", "KEY2").size(), 2);
  ASSERT_EQ(config.Get("SECTION2", "KEY3").size(), 3);

  EXPECT_EQ(config.Get("SECTION2", "KEY1")[0], "value1");
  EXPECT_EQ(config.Get("SECTION2", "KEY2")[0], "value1");
  EXPECT_EQ(config.Get("SECTION2", "KEY2")[1], "value2");
  EXPECT_EQ(config.Get("SECTION2", "KEY3")[0], "value1");
  EXPECT_EQ(config.Get("SECTION2", "KEY3")[1], "value2");
  EXPECT_EQ(config.Get("SECTION2", "KEY3")[2], "value3");
}

TEST_F(IniParserTests, non_string_values) {
  std::string ini_data = R"(
  [section1]
  key1 = 1
  key2 = 1.0
  flag = true
  flag2 = false 
  [section2]
  key1 = { 
    1, 
    2, 
    3
  }
  key2 = { 1.0, 2.0, 3.0, 4.0 }
  )";

  IniFileParser parser;
  ConfigTable config;
  ASSERT_NO_THROW(config = parser.Parse(ini_data));

  ASSERT_TRUE(config.SectionExists("SECTION1"));
  ASSERT_TRUE(config.SectionExists("SECTION2"));

  EXPECT_EQ(config.Get("SECTION1").size(), 4);
  EXPECT_EQ(config.Get("SECTION2").size(), 2);

  ASSERT_TRUE(config.KeyExists("SECTION1", "KEY1"));
  ASSERT_TRUE(config.KeyExists("SECTION1", "KEY2"));
  ASSERT_TRUE(config.KeyExists("SECTION1", "FLAG"));
  ASSERT_TRUE(config.KeyExists("SECTION1", "FLAG2"));

  ASSERT_TRUE(config.KeyExists("SECTION2", "KEY1"));
  ASSERT_TRUE(config.KeyExists("SECTION2", "KEY2"));

  auto key1 = config.GetVal<int32_t>("SECTION1", "KEY1");
  auto key2 = config.GetVal<float>("SECTION1", "KEY2");
  auto flag = config.GetVal<bool>("SECTION1", "FLAG");
  auto flag2 = config.GetVal<bool>("SECTION1", "FLAG2");

  ASSERT_TRUE(key1.has_value());
  EXPECT_EQ(key1.value(), 1);

  ASSERT_TRUE(key2.has_value());
  EXPECT_EQ(key2.value(), 1.0f);

  ASSERT_TRUE(flag.has_value());
  EXPECT_EQ(flag.value(), true);

  ASSERT_TRUE(flag2.has_value());
  EXPECT_EQ(flag2.value(), false);

  auto key1_2 = config.GetVal<std::vector<uint32_t>>("SECTION2", "KEY1");
  auto key2_2 = config.GetVal<std::vector<float>>("SECTION2", "KEY2");

  ASSERT_TRUE(key1_2.has_value());
  EXPECT_EQ(key1_2.value().size(), 3);

  EXPECT_EQ(key1_2.value()[0], 1);
  EXPECT_EQ(key1_2.value()[1], 2);
  EXPECT_EQ(key1_2.value()[2], 3);

  ASSERT_TRUE(key2_2.has_value());
  EXPECT_EQ(key2_2.value().size(), 4);
  EXPECT_EQ(key2_2.value()[0], 1.0f);
  EXPECT_EQ(key2_2.value()[1], 2.0f);
  EXPECT_EQ(key2_2.value()[2], 3.0f);
}

TEST_F(IniParserTests, trailing_commas) {
  std::string ini_data = R"(
  [section1]
  key1 = { 1, 2, 3, }
  key2 = { 1.0, 2.0, 3.0, 4.0, }
  )";

  IniFileParser parser;
  ConfigTable config;
  ASSERT_NO_THROW(config = parser.Parse(ini_data));

  ASSERT_TRUE(config.SectionExists("SECTION1"));

  EXPECT_EQ(config.Get("SECTION1").size(), 2);

  ASSERT_TRUE(config.KeyExists("SECTION1", "KEY1"));
  ASSERT_TRUE(config.KeyExists("SECTION1", "KEY2"));

  auto key1 = config.GetVal<std::vector<uint32_t>>("SECTION1", "KEY1");
  auto key2 = config.GetVal<std::vector<float>>("SECTION1", "KEY2");

  ASSERT_TRUE(key1.has_value());
  EXPECT_EQ(key1.value().size(), 3);

  EXPECT_EQ(key1.value()[0], 1);
  EXPECT_EQ(key1.value()[1], 2);
  EXPECT_EQ(key1.value()[2], 3);

  ASSERT_TRUE(key2.has_value());
  EXPECT_EQ(key2.value().size(), 4);
  EXPECT_EQ(key2.value()[0], 1.0f);
  EXPECT_EQ(key2.value()[1], 2.0f);
  EXPECT_EQ(key2.value()[2], 3.0f);
  EXPECT_EQ(key2.value()[3], 4.0f);
}

TEST_F(IniParserTests, simple_comments) {
  std::string ini_data = R"(
  [section1]
  key1 = value1 # comment

  #[
    multiline comment
    with multiple lines
  #]
  [section2] # comment
  #[
    weirdly placed multiline comment
  #] # with a weird comment after
  key1 = value1
  )";

  IniFileParser parser;
  ConfigTable config;
  ASSERT_NO_THROW(config = parser.Parse(ini_data));

  ASSERT_TRUE(config.SectionExists("SECTION1"));
  ASSERT_TRUE(config.SectionExists("SECTION2"));

  EXPECT_EQ(config.Get("SECTION1").size(), 1);
  EXPECT_EQ(config.Get("SECTION2").size(), 1);

  ASSERT_TRUE(config.KeyExists("SECTION1", "KEY1"));
  ASSERT_TRUE(config.KeyExists("SECTION2", "KEY1"));
}
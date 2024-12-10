/**
 * \file fuzzer_tests.cpp
 **/
#include <string>
#include <vector>

#include "oetest.hpp"

// #include "fuzzing/fuzzer.hpp"

using namespace other;
class FuzzerTests : public OtherTest {
 public:
};

// TEST_F(FuzzerTests, produce_specified_grammar) {
//   fuzz::FuzzingContext ctx;
//   Ref<fuzz::Fuzzer> fuzzer = fuzz::Char();
//   Ref<fuzz::Fuzzer> fuzzer2 = fuzz::String(100);

//   (*fuzzer)(ctx);
//   ctx.parse_input += ":";
//   (*fuzzer2)(ctx);
//   std::cout << "COMPLETELY RANDOM: ";
//   std::cout << ctx.parse_input.Finalize() << std::endl;
//   /// 100 characters + ':' + character
//   ASSERT_LT(ctx.parse_input.Finalize().length(), 103);

//   fuzz::FuzzingContext ctx2;
//   std::vector<std::string> choices{ "one", "two", "or", "THREE" };
//   Ref<fuzz::Fuzzer> fuzzer3 = fuzz::ChooseFrom(choices);
//   (*fuzzer3)(ctx2);

//   ASSERT_NE(std::ranges::find(choices, (std::string)ctx2.parse_input), choices.end());

//   std::cout << "-----------------------------\n";
//   std::cout << ctx2.parse_input.Finalize() << std::endl;

//   fuzz::FuzzingContext ctx3;
//   Ref<fuzz::Fuzzer> fuzzer4 = fuzz::Produce('[') >> fuzzer3 >> fuzz::Produce(']');
//   (*fuzzer4)(ctx3);
//   std::string res = ctx3.parse_input;
//   ASSERT_TRUE(res.starts_with('['));
//   ASSERT_TRUE(res.ends_with(']'));
//   ASSERT_FALSE(res.contains(' '));

//   res.erase(std::find_if(res.rbegin(), res.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), res.end());
//   res.erase(std::find_if(res.begin(), res.end(), [](unsigned char ch) { return !std::isspace(ch); }));

//   ASSERT_NE(std::ranges::find(choices, (std::string)ctx2.parse_input), choices.end());

//   std::cout << "-------------------------------\n";
//   std::cout << (std::string)ctx3.parse_input << std::endl;
// }

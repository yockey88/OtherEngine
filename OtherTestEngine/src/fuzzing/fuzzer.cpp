/**
 * \file fuzzing/fuzzer.cpp
 **/
#include "fuzzing/fuzzer.hpp"

namespace other {
namespace fuzz {
    
  void FuzzOutput::Add(const std::string_view string) {
    result << std::string{ string };
  }
    
  FuzzOutput::operator std::string() {
    return Finalize();
  }
    
  std::string FuzzOutput::Finalize() {
    return result.str();
  }

  void ProduceChar::operator()(FuzzingContext& context) {
    context.parse_input += ch;
  }
  
  Ref<Fuzzer> Produce(char c) {
    return NewRef<ProduceChar>(c);
  }

  void FuzzChar::operator()(FuzzingContext& context) {
    uint16_t random_int = 0;

    do {
      random_int = context.random.Next();
    } while (random_int < 'A'  || random_int > 'z' ||  
             (random_int > 'Z' && random_int < 'a'));

    context.parse_input += char(random_int);
  }

  void FuzzCharGroup::operator()(FuzzingContext& context) {
    uint16_t character = context.random.Next() % char_choices.length();
    context.parse_input += char_choices[character];
  }

  Ref<Fuzzer> Char() {
    return NewRef<FuzzChar>();
  }

  Ref<Fuzzer> CharFrom(const std::string_view group) {
    return NewRef<FuzzCharGroup>(group);
  }

  void FuzzString::operator()(FuzzingContext& context) {
    Ref<FuzzChar> character = Char();

    uint16_t len = context.random.Next();
    if (length != size_t(-1)) {
      len %= length;
    }

    for (uint16_t i = 0; i < len; ++i) {
      (*character)(context);
    }
  }

  void FuzzStringGroup::operator()(FuzzingContext& context) {
    uint16_t random_str = context.random.Next() % choices.size();
    context.parse_input.Add(choices[random_str]);
  }

  Ref<Fuzzer> String(size_t length) {
    return NewRef<FuzzString>(length);
  }
  
  Ref<Fuzzer> ChooseFrom(const std::vector<std::string>& group) {
    return NewRef<FuzzStringGroup>(group);
  }
    
  void SequentialFuzzer::operator()(FuzzingContext& context) {
    (*first)(context);
    if (delim.has_value()) {
      context.parse_input.Add(*delim);
    }

    (*second)(context);
    if (delim.has_value()) {
      context.parse_input.Add(*delim);
    }
  }

  Ref<Fuzzer> ChainWithDelim(const std::string_view delim, const Ref<Fuzzer>& first, const Ref<Fuzzer>& second) {
    return NewRef<SequentialFuzzer>(delim, first, second);
  }

  Ref<Fuzzer> operator>>(const Ref<Fuzzer>& first, const Ref<Fuzzer>& second) {
    return NewRef<SequentialFuzzer>(first, second);
  }

} // namespace fuzz
}  // namespace other

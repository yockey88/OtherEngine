/**
 * \file fuzzing/fuzzer.hpp
 **/
#ifndef OTHER_TEST_ENGINE_FUZZER_HPP
#define OTHER_TEST_ENGINE_FUZZER_HPP

#include <cstdint>
#include <sstream>
#include <string_view>

#include "core/defines.hpp"
#include "core/meta.hpp"
#include "core/rand.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"

namespace other {
  namespace fuzz {

    /// TODO: replace this with something that allows for the production of
    ///       arbitrary types for fuzzing arbitrary parts of engine
    template <typename T>
    concept Accumulatable = is_container<T>;

    struct FuzzOutput {
      template <typename T>
        requires requires(T t) {
          std::declval<std::stringstream>() << t;
        }
      std::stringstream& operator+=(T val) {
        result << val;
        return result;
      }

      void Add(const std::string_view string);

      operator std::string();
      std::string Finalize();

     private:
      std::stringstream result;
    };

    struct FuzzingContext {
      RandomGenerator<uint16_t> random;
      FuzzOutput parse_input;
    };

    class Fuzzer : public RefCounted {
     public:
      Fuzzer() {}
      virtual ~Fuzzer() {}

      virtual void operator()(FuzzingContext& context) = 0;

     protected:
    };

    struct ProduceChar : Fuzzer {
      char ch;

      ProduceChar(char c)
          : ch(c) {}

      virtual ~ProduceChar() override {}

      void operator()(FuzzingContext& context) override;
    };

    Ref<Fuzzer> Produce(char c);

    struct FuzzChar : Fuzzer {
      FuzzChar() = default;
      virtual ~FuzzChar() override {}

      void operator()(FuzzingContext& context) override;
    };

    struct FuzzCharGroup : Fuzzer {
      std::string char_choices;

      FuzzCharGroup(const std::string_view choices)
          : char_choices(choices) {}
      virtual ~FuzzCharGroup() override {}

      void operator()(FuzzingContext& context) override;
    };

    Ref<Fuzzer> Char();
    Ref<Fuzzer> CharFrom(const std::string_view group);

    struct FuzzString : Fuzzer {
      size_t length;

      FuzzString(size_t length)
          : length(length) {}
      virtual ~FuzzString() override {}

      void operator()(FuzzingContext& context) override;
    };

    struct FuzzStringGroup : Fuzzer {
      std::vector<std::string> choices;

      FuzzStringGroup(const std::vector<std::string>& choices)
          : choices(choices) {}
      virtual ~FuzzStringGroup() override {}

      void operator()(FuzzingContext& context) override;
    };

    Ref<Fuzzer> String(size_t length = size_t(-1));
    Ref<Fuzzer> ChooseFrom(const std::vector<std::string>& group);

    struct SequentialFuzzer : Fuzzer {
      Opt<std::string> delim;
      Ref<Fuzzer> first;
      Ref<Fuzzer> second;

      SequentialFuzzer(const Ref<Fuzzer>& first, const Ref<Fuzzer>& second)
          : first(first), second(second) {}
      SequentialFuzzer(const std::string_view delim, const Ref<Fuzzer>& first, const Ref<Fuzzer>& second)
          : delim(delim), first(first), second(second) {}
      virtual ~SequentialFuzzer() override {}

      void operator()(FuzzingContext& context) override;
    };

    Ref<Fuzzer> ChainWithDelim(const std::string_view delim, const Ref<Fuzzer>& first, const Ref<Fuzzer>& second);

    Ref<Fuzzer> operator>>(const Ref<Fuzzer>& first, const Ref<Fuzzer>& second);

  }  // namespace fuzz
}  // namespace other

#endif  // !OTHER_TEST_ENGINE_FUZZER_HPP

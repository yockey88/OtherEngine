/**
 * \file fuzzing.cpp
 **/
#include "fuzzing/fuzzer.hpp"
#include "testing_core/test.hpp"

#include "mock_engine.hpp"
#include "other_engine.hpp"

using namespace other;

class FuzzingApp : public MockApp {
 public:
  FuzzingApp(const CmdLine& cmd_line, const ConfigTable& config)
      : MockApp(cmd_line, config) {}
  virtual ~FuzzingApp() override {}

  TestDescription GetTestDescription() override {
    return { "Fuzzing-Test" };
  }
};

OTHER_ENTRY_POINT(FuzzingApp);
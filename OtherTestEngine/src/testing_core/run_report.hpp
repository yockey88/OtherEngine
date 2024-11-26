/**
 * \file testing_core/run_report.hpp
 **/
#ifndef OTHER_TEST_ENGINE_RUN_REPORT_HPP
#define OTHER_TEST_ENGINE_RUN_REPORT_HPP

#include <nlohmann/json.hpp>

#include "core/formatters.hpp"
#include "core/writer_reader.hpp"

namespace other {

  enum class RunResult {
    TEST_SUCCESS = 0,
    TEST_FAILURE,
    TEST_INCONCLUSIVE
  };

  struct RunReport {
    std::string_view test_name = "";
    uint32_t num_warnings = 0;
    uint32_t num_errors = 0;
    uint32_t num_tests = 0;
    RunResult result = RunResult::TEST_INCONCLUSIVE;
  };

  template <>
  struct Writer<RunReport> {
    nlohmann::json operator()(const RunReport& report) const {
      nlohmann::json j = {
        { "test_name", report.test_name },
        { "num_warnings", report.num_warnings },
        { "num_errors", report.num_errors },
        { "num_tests", report.num_tests },
        { "result", fmtstr("{}", report.result) },
      };

      return j;
    }
  };

}  // namespace other

#endif  // !OTHER_TEST_ENGINE_RUN_REPORT_HPP
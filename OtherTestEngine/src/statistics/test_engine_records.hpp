/**
 * \file statistices/test_engine_records.hpp
 **/
#ifndef OTHER_TEST_ENGINE_RECORDER_HPP
#define OTHER_TEST_ENGINE_RECORDER_HPP

#include <nlohmann/json_fwd.hpp>

#include "core/directory.hpp"
#include "core/file_handle.hpp"

namespace other {

  class TestEngineRecords {
   public:
    enum InformationCategory {
      ERROR_DESCRIPTION = 0,
      BENCHMARK_DATA,
      TEST_DATA,
      RUN_REPORT,

      NUM_INFORMATION_CATEGORIES,
      INVALID_INFORMATION_CATEGORY = NUM_INFORMATION_CATEGORIES
    };

    struct Paths {
      Path paths[NUM_INFORMATION_CATEGORIES] = {
        "errors.json",
        "benchmarks.json",
        "tests.json",
        "engine_run_data.json"
      };
    } paths;

    TestEngineRecords();
    TestEngineRecords(const Paths& paths);
    ~TestEngineRecords();

    template <typename T>
      requires JsonWritable<T>
    void Record(InformationCategory category, const T& data) {
      try {
        nlohmann::json j = Writer<T>{}(data);
        json_data[category].push_back(j);
      } catch (...) {
        return;
      }
    }

   private:
    Ref<Directory> test_data_dir = nullptr;
    Ref<FileHandle> file_handles[NUM_INFORMATION_CATEGORIES] = { nullptr };
    nlohmann::json json_data[NUM_INFORMATION_CATEGORIES] = {};

    void Initialize();

    void OpenFile(const Path& path, InformationCategory category);
    void CloseFile(InformationCategory category);
  };

}  // namespace other

#endif  // !OTHER_TEST_ENGINE_RECORDER_HPP
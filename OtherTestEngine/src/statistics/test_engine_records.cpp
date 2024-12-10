/**
 * \file statistics/test_engine_records.cpp
 **/
#include "statistics/test_engine_records.hpp"

#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json_fwd.hpp>

#include "core/filesystem.hpp"
#include "core/logger.hpp"

namespace other {

  TestEngineRecords::TestEngineRecords() {
    Initialize();
  }

  TestEngineRecords::TestEngineRecords(const Paths& paths)
      : paths(paths) {
    Initialize();
  }

  TestEngineRecords::~TestEngineRecords() {
    CloseFile(InformationCategory::ERROR_DESCRIPTION);
    CloseFile(InformationCategory::BENCHMARK_DATA);
    CloseFile(InformationCategory::TEST_DATA);
    CloseFile(InformationCategory::RUN_REPORT);
  }

  void TestEngineRecords::Initialize() {
    Ref<Directory> directory = Filesystem::ProjectDirectory();
    if (directory == nullptr) {
      println("Failed to get project directory");
      return;
    }

    test_data_dir = directory->AddFolder("test_data");
    if (test_data_dir == nullptr || !test_data_dir->Exists()) {
      println("Failed to create test data directory");
      return;
    }

    for (uint32_t i = 0; i < InformationCategory::NUM_INFORMATION_CATEGORIES; i++) {
      InformationCategory category = static_cast<InformationCategory>(i);
      OpenFile(paths.paths[i], category);
    }
  }

  void TestEngineRecords::OpenFile(const Path& path, InformationCategory category) {
    if (test_data_dir == nullptr || !test_data_dir->Exists()) {
      println("Failed to open file : {}", path.string());
      return;
    }
    file_handles[category] = test_data_dir->GetFile(path);
    if (file_handles[category] == nullptr) {
      println("Failed to get file handle : {}", path.string());
      return;
    }

    try {
      file_handles[category]->Open(std::ios_base::in);
      json_data[category] = nlohmann::json::parse(file_handles[category]->ReadString());
      file_handles[category]->Close();
    } catch (nlohmann::json::parse_error& e) {
    } catch (...) {
    }
  }

  void TestEngineRecords::CloseFile(InformationCategory category) {
    if (file_handles[category] == nullptr || !file_handles[category]->Exists()) {
      println("Failed to close file : {}", paths.paths[category].string());
      return;
    }

    file_handles[category]->Open(std::ios_base::in | std::ios_base::out | std::ios::app);
    file_handles[category]->WriteJson(json_data[category]);
    file_handles[category]->Close();
    file_handles[category] = nullptr;
  }

}  // namespace other
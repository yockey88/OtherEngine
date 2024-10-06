/**
 * \file unit_tests/oetest.cpp
 **/
#include "unit_tests/oetest.hpp"

#include "core/logger.hpp"
#include "parsing/ini_parser.hpp"
#include "scripting/script_engine.hpp"

namespace other {

ConfigTable OtherTest::config = {};
ConfigTable OtherTest::stashed_config = {};

CmdLine OtherTest::cmdline = {};
    
void OtherTest::SetUpTestSuite() {  
  const std::string configpath = "C:/Yock/code/OtherEngine/tests/unit_tests/unittest.other";      
  IniFileParser parser(configpath);
  try {
    config = parser.Parse();
  } catch (IniException& e) {
    FAIL() << "Failed to parse INI file for unit test!\n\t" << e.what();
    return;
  }
}

void OtherTest::TearDownTestSuite() {
}

void OtherTest::SetUp() {
  OpenLog();
}

void OtherTest::TearDown() {
  CloseLog();
}

void OtherTest::OpenLog() {
  if (Logger::IsOpen()) {
    return;
  }

  /// FIXME: customize config per unit test but somehow not create bajillions of config files
  Logger::Open(config);
  Logger::Instance()->RegisterThread("Main Test Thread");
}

void OtherTest::CloseLog() {
  Logger::Shutdown();
}

bool CheckNumScripts(uint32_t cs , uint32_t lua , uint32_t python) {
  auto& modules = ScriptEngine::GetModules();
  for (auto& module : modules) {
    auto& scripts = module.second.module->GetModules();

    switch (module.second.module->GetLanguageType()) {
      case LanguageModuleType::CS_MODULE:
        return scripts.size() == cs;
      case LanguageModuleType::LUA_MODULE:
        return scripts.size() == lua;
      case LanguageModuleType::PYTHON_MODULE:
        return scripts.size() == python;
      default:
        return false;
    }
  }
  return false;
}

} // namespace other

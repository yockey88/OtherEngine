/**
 * \file mock_engine.cpp
 **/
#include "mock_engine.hpp"

#include <stdexcept>

#include "core/logger.hpp"
#include "core/errors.hpp"
#include "parsing/ini_parser.hpp"

#include "input/io.hpp"
#include "event/event_queue.hpp"

#include "scripting/script_engine.hpp"

namespace other {

  MockEngine* MockEngine::instance = nullptr;
      
  MockEngine::MockEngine(const Path& configpath) {
    other::IniFileParser parser(configpath.string());
    try {
      config = parser.Parse();
    } catch (other::IniException& e) {
      throw std::runtime_error(fmtstr("INI Parsing failure {}" , e.what()));
    }

    instance = this;
  } 

  MockEngine::~MockEngine() {}

  MockEngine* MockEngine::TestEngine() { 
    if (instance == nullptr) {
      throw std::logic_error("Cannot call MockEngine::TestEngine until after Gtest has begun!");
    }
    /// registered on gtest entry
    return instance;
  }

  void MockEngine::SetUp() {
    other::Logger::Open(config);
    other::Logger::Instance()->RegisterThread("Main Other Engine Test Engine Thread");
    OE_TRACE("Logger initialized for unit test");

    InitializeEngineSubSystems();
    OE_TRACE("Engine sub-systems initialized");
  }

  void MockEngine::TearDown() {
    OE_TRACE("Engine sub-systems shutdown");
    ShutdownEngineSubSystems();

    OE_TRACE("Shutting down logger for unit test");
    other::Logger::Shutdown();
  }
      
  void MockEngine::InitializeEngineSubSystems() {
    IO::Initialize();
    EventQueue::Initialize(config);
    ScriptEngine::Initialize(config);
  }

  void MockEngine::ShutdownEngineSubSystems() {
    ScriptEngine::Shutdown();
    EventQueue::Shutdown();
    IO::Shutdown();
  }

} // namespace other

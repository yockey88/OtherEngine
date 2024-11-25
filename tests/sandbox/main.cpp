/**
 * \file sandbox/main.cpp
 **/
#include "other_engine.hpp"

using namespace other;

class SandboxApp : public App {
 public:
  SandboxApp(const other::CmdLine& cmd_line, const other::ConfigTable& config)
      : other::App(cmd_line, config) {}
  virtual ~SandboxApp() override {}

  virtual void OnAttach() override {
    OE_INFO("Sandbox App Attached");
  }
};

OTHER_ENTRY_POINT(SandboxApp);
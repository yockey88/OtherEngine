/**
 * \file sandbox/main.cpp
 **/
#include "other_engine.hpp"

class SandboxApp : public other::App {
 public:
  SandboxApp(const other::CmdLine& cmd_line, const other::ConfigTable& config)
      : other::App(cmd_line, config) {}
  virtual ~SandboxApp() override {}
};

OTHER_ENTRY_POINT(SandboxApp);

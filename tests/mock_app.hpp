/**
 * \file tests/mock_app.hpp
 **/
#ifndef OTHER_ENGINE_MOCK_APP_HPP
#define OTHER_ENGINE_MOCK_APP_HPP

#include "application/app.hpp"

class TestApp : public other::App {
  public:
    TestApp(const other::CmdLine& cmd_line, const other::ConfigTable& config) 
      : other::App(cmd_line, config) {}
    virtual ~TestApp() override {}
};

namespace other {
Scope<App> NewApp(const CmdLine& cmd_line, const ConfigTable& config);
} // namespace other

void CheckGlError(int line);

#define CHECK() do { CheckGlError(__LINE__); } while (false)

#endif // !OTHER_ENGINE_MOCK_APP_HPP

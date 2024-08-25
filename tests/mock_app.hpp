/**
 * \file tests/mock_app.hpp
 **/
#ifndef OTHER_ENGINE_MOCK_APP_HPP
#define OTHER_ENGINE_MOCK_APP_HPP

#include "application/app.hpp"

class TestApp : public other::App {
  public:
    TestApp(other::Engine* engine) 
        : other::App(engine) {}
    virtual ~TestApp() override {}
};

namespace other {
  
  Scope<App> NewApp(Engine* engine);

} // namespace other

void CheckGlError(int line);

#define CHECK() do { CheckGlError(__LINE__); } while (false)

#endif // !OTHER_ENGINE_MOCK_APP_HPP

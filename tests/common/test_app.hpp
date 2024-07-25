/**
 * \file common/test_app.hpp
 **/
#ifndef OTHER_ENGINE_TEST_APP_HPP
#define OTHER_ENGINE_TEST_APP_HPP

#include "application/app.hpp"

class TestApp : public other::App {
  public:
    TestApp(other::Engine* engine) 
        : other::App(engine) {}
    virtual ~TestApp() override {}
};

#endif // !OTHER_ENGINE_TEST_APP_HPP

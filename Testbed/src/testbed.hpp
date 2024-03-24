/**
 * \file testbed.hpp
*/
#ifndef OTHER_ENGINE_TESTBED_HPP
#define OTHER_ENGINE_TESTBED_HPP

#include "application/app.hpp"

namespace other {

  class Testbed : public App {
    public:
      Testbed(Engine* engine , const ConfigTable& cfg) 
        : App(engine , cfg) {}
      virtual ~Testbed() override {}

      virtual void OnAttach() override;
      virtual void OnEvent(Event* event) override;
      virtual void Update(float dt) override {}
      virtual void Render() override {}
      virtual void RenderUI() override {}
      virtual void OnDetach() override {}
  };

} // namespace other

#endif // !OTHER_ENGINE_TESTBED_HPP

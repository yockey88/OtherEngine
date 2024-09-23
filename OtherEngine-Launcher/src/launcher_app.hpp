/**
 * \file launcher_app.hpp
 **/
#ifndef OTHER_ENGINE_LAUNCHER_APP_HPP
#define OTHER_ENGINE_LAUNCHER_APP_HPP

#include "application/app.hpp"

namespace other {

  class LauncherApp : public other::App {
    public:
      LauncherApp(const CmdLine& cmd_line, const ConfigTable& config) 
        : other::App(cmd_line, config) {}
      virtual ~LauncherApp() override {}

    private:      
      virtual void OnAttach() override;
      virtual void OnEvent(other::Event* event) override;
      virtual void Update(float dt) override {}
      virtual void Render() override {}
      virtual void RenderUI() override {}
      virtual void OnDetach() override {}
  };

} // namespace other

#endif // !OTHER_ENGINE_LAUNCHER_APP_HPP

/** \file core/engine.hpp
 */
#ifndef OTHER_ENGINE_ENGINE_HPP
#define OTHER_ENGINE_ENGINE_HPP

#include "core/defines.hpp"
#include "core/layer_stack.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "event/event_queue.hpp"
#include "application/app.hpp"
#include "asset/asset_handler.hpp"

namespace other {

  class Engine {
    public:
      Opt<ExitCode> exit_code = std::nullopt;

      Engine() = default;
      Engine(const CmdLine& cmd_line , const ConfigTable& config , const std::string& config_path);

      static Engine Create(const CmdLine& cmd_line , const ConfigTable& config , const std::string& config_path);

      void LoadApp(Scope<App>& app);
      Scope<App>& ActiveApp() { return active_app; }
      void UnloadApp();

      void ProcessEvent(Event* event);

      Logger* GetLog() const { return logger_instance; }

      CmdLine cmd_line;
      ConfigTable config;
    private:
      std::string config_path;

      Logger* logger_instance = nullptr;

      Ref<Project> project_metadata;

      Scope<App> active_app;
      Scope<AssetHandler> asset_handler;
  };

} // namespace other

#endif // !OTHER_ENGINE_ENGINE_HPP

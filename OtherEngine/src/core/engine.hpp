/** \file core/engine.hpp
 */
#ifndef OTHER_ENGINE_ENGINE_HPP
#define OTHER_ENGINE_ENGINE_HPP

#include "core/defines.hpp"
#include "parsing/cmd_line_parser.hpp"

#include "application/app.hpp"

namespace other {

  class Event;
  
  /// implemented by client 
#ifndef OTHERENGINE_DLL
  extern Scope<App> NewApp(const CmdLine& cmd_line, const ConfigTable& config);
#endif // !OTHERENGINE_DLL

  class Engine {
    public:
      Engine();
      Engine(const CmdLine& cmd_line);

      ExitCode Run();
      
      void LoadApp();
      void UnloadApp();

      void Launch();
      void Shutdown();

      void ProcessEvent(Event* event);

      CmdLine cmd_line;
      ConfigTable config;
    private:
      std::string config_path;

      Scope<App> active_app;

      Opt<Path> FindConfigFile();
      ExitCode ProcessExitCode(ExitCode code);
      ExitCode LoadConfig();

      void ProcessSingleArg(const std::string_view lflag , uint32_t min_args , std::function<bool(Arg&)> processor = nullptr);
  };

} // namespace other

#endif // !OTHER_ENGINE_ENGINE_HPP

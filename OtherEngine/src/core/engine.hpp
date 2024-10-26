/** \file core/engine.hpp
 */
#ifndef OTHER_ENGINE_ENGINE_HPP
#define OTHER_ENGINE_ENGINE_HPP

#include "core/defines.hpp"
#include "core/time.hpp"

#include "application/app.hpp"
#include "parsing/cmd_line_parser.hpp"

namespace other {

  /// implemented by client
#ifndef OTHERENGINE_DLL
  extern App* NewApp(const CmdLine& cmd_line, const ConfigTable& config);
#else
  OE_API App* NewApp(const CmdLine& cmd_line, const ConfigTable& config);
#endif  // !OTHERENGINE_DLL

  class Engine {
   public:
    Engine();
    Engine(const CmdLine& cmd_line);

    ExitCode Run();

    void LoadApp();
    void UnloadApp();

    void Launch();
    void Shutdown();

    void Start();
    void Tick();
    void Stop();

    CmdLine cmd_line;
    ConfigTable config;

   private:
    time::DeltaTime delta;
    std::string config_path;

    Opt<Path> FindConfigFile();
    ExitCode ProcessExitCode(ExitCode code);
    ExitCode LoadConfig();

    void PushCoreLayer();
    void RegisterLoggers();

    void ProcessSingleArg(const std::string_view lflag, uint32_t min_args, std::function<bool(Arg&)> processor = nullptr);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ENGINE_HPP

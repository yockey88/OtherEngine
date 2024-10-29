/** \file engine/engine.hpp
 */
#ifndef OTHER_ENGINE_ENGINE_HPP
#define OTHER_ENGINE_ENGINE_HPP

#include <queue>

#include "core/defines.hpp"
#include "core/state.hpp"
#include "core/time.hpp"

#include "application/app.hpp"
#include "event/core_events.hpp"
#include "parsing/cmd_line_parser.hpp"

#include "engine/engine_state_machine.hpp"

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
    Engine(const CmdLine& cmd_line, Opt<std::string> main_thread_name = std::nullopt);
    ~Engine();

    void Start();
    void Step();
    void Stop();

    void EngineEvent(EngineStateEvent event);

    CmdLine cmd_line;
    ConfigTable config;
    Opt<ExitCode> exit_code = std::nullopt;

   private:
    time::DeltaTime delta;
    std::string config_path;

    std::queue<EngineStateEvent> event_queue;
    Ref<EngineStateMachine> state = nullptr;

    Opt<Path> FindConfigFile();
    ExitCode ProcessExitCode(ExitCode code);
    ExitCode LoadConfig();

    // bool HandleShutdownEvent(ShutdownEvent& event);

    void ProcessSingleArg(const std::string_view lflag, uint32_t min_args, std::function<bool(Arg&)> processor = nullptr);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ENGINE_HPP

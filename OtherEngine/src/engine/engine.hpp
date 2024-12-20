/** \file engine/engine.hpp
 */
#ifndef OTHER_ENGINE_ENGINE_HPP
#define OTHER_ENGINE_ENGINE_HPP

#include <queue>

#include "core/defines.hpp"
#include "core/time.hpp"
#include "engine/engine_state_machine.hpp"

#include "application/app.hpp"
#include "event/core_events.hpp"
#include "parsing/cmd_line_parser.hpp"

namespace other {

  CLIENT_SIDE App* NewApp(const CmdLine& cmd_line, const ConfigTable& config);
  CLIENT_SIDE Engine* LoadDriver(const CmdLine& cmd_line);

  CLIENT_SIDE void FreeApp(App* app);
  CLIENT_SIDE void UnloadDriver(Engine* driver);

  class Engine {
   public:
    Engine();
    Engine(const CmdLine& cmd_line, std::string main_thread_name = "OtherEngine--MainThread");
    virtual ~Engine();

    virtual void Run();

    bool IsRunning() const;

    void EngineEvent(EngineStateEvent event);

    CmdLine cmd_line;
    ConfigTable config;
    Opt<ExitCode> exit_code = std::nullopt;

    float dt = 0.0f;

   protected:
    virtual void Start();
    virtual void Step();
    virtual void Stop();
    virtual Ref<EngineStateMachine> CreateStateMachine();

    time::DeltaTime delta;
    std::string config_path;

    std::queue<EngineStateEvent> event_queue;
    Ref<EngineStateMachine> state = nullptr;

    Opt<Path> FindConfigFile();
    ExitCode LoadConfig();

    bool HandleShutdownEvent(ShutdownEvent& event);
    void ProcessSingleArg(const std::string_view lflag, uint32_t min_args, std::function<bool(Arg&)> processor = nullptr);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ENGINE_HPP

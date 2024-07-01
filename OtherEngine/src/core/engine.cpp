/**
 * \file core/engine.cpp
 */
#include "core/engine.hpp"

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "core/config.hpp"

#include "event/event.hpp"
#include "event/core_events.hpp"
#include "event/event_handler.hpp"

#include "editor/editor.hpp"

namespace other {

  Engine::Engine(const CmdLine& cmdline , const ConfigTable& config , const std::string& config_path) 
      : cmd_line(cmdline) , config(config) , config_path(config_path) {
    logger_instance = Logger::Instance();
  }

  Engine Engine::Create(const CmdLine& cmd_line , const ConfigTable& config , const std::string& config_path) {
    return Engine(cmd_line , config , config_path);
  }

  void Engine::LoadApp(Scope<App>& app) {
    OE_ASSERT(app != nullptr , "Attempting to load a null application");

    println("Loading application");
    if (cmd_line.HasFlag("--editor")) {
      println("Loading editor");
      auto editor_app = NewScope<Editor>(this , app);
      active_app = std::move(editor_app);
    } else {
      active_app = std::move(app);
    }

    active_app->Load();
    /// now allowed to use logger
  }

  void Engine::UnloadApp() {
    OE_ASSERT(active_app != nullptr , "Attempting to unload a null application");

    active_app->Unload();
    active_app = nullptr;

    OE_DEBUG("Engine unloaded");
  }

  void Engine::ProcessEvent(Event* event) {
    OE_ASSERT(event != nullptr , "Attempting to process a null event");
    
    if (event->handled) {
      return;
    }

    EventHandler handler(event);
    handler.Handle<ShutdownEvent>([this](ShutdownEvent& sd) -> bool { 
      exit_code = sd.GetExitCode(); 
      return true;
    });
  }

} // namespace other

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

#include "application/app_state.hpp"
#include "application/runtime_layer.hpp"

#include "editor/editor.hpp"
#include "editor/editor_layer.hpp"
#include "event/event_queue.hpp"
#include "editor/editor_console_sink.hpp"

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
    
    in_editor = cmd_line.HasFlag("--editor");
    if (in_editor) {
      auto editor_app = NewScope<Editor>(this);
      active_app = std::move(editor_app);
    } else {
      active_app = std::move(app);
    }
      
    active_app->Load(); 
    AppState::Initialize(active_app.get() , active_app->layer_stack , active_app->scene_manager , 
                         active_app->asset_handler , active_app->project_metadata);
  }
      
  void Engine::PushCoreLayer() {
    Ref<Layer> core_layer = nullptr;
    if (in_editor) {
      core_layer = NewRef<EditorLayer>(active_app.get());
    } else {
      core_layer = NewRef<RuntimeLayer>(active_app.get() , active_app->config);
    }
    active_app->PushLayer(core_layer);

    if (in_editor) {
      logger_instance->RegisterTarget({
        .target_name = "Editor-Console" ,
        .level = Logger::LevelFromLevel(Logger::Level::DEBUG) ,
        .log_format = "%v" ,
        .sink_factory = []() -> spdlog::sink_ptr {
          return NewStdRef<EditorConsoleSink>(10); 
        } , 
      });
    }

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

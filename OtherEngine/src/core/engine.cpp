/**
 * \file core/engine.cpp
 */
#include "core/engine.hpp"

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "event/event.hpp"
#include "event/core_events.hpp"
#include "editor/editor.hpp"

namespace other {

  Engine::Engine(const CmdLine& cmdline , const ConfigTable& config , const std::string& config_path) 
      : cmd_line(cmdline) , config(config) , config_path(config_path) {
    layer_stack = NewScope<LayerStack>();
    asset_handler = NewScope<AssetHandler>();
    logger_instance = Logger::Instance();
  }

  Engine Engine::Create(const CmdLine& cmd_line , const ConfigTable& config , const std::string& config_path) {
    return Engine(cmd_line , config , config_path);
  }

  void Engine::LoadApp(Scope<App>& app) {
    OE_ASSERT(app != nullptr , "Attempting to load a null application");

    OE_DEBUG("Loading application");
    if (cmd_line.HasFlag("--editor")) {
      OE_DEBUG("Loading editor");

      auto editor_app = NewScope<Editor>(this , app);
      active_app = std::move(editor_app);
      active_app->OnLoad();

      return;
    } else {
      active_app = std::move(app);
      active_app->OnLoad();
    }

  }

  void Engine::UnloadApp() {
    OE_ASSERT(active_app != nullptr , "Attempting to unload a null application");

    active_app->OnUnload();
    active_app = nullptr;
  }

  void Engine::ProcessEvent(Event* event) {
    if (event->handled) {
      return;
    }

    for (auto itr = layer_stack->end(); itr != layer_stack->begin();) {
      (*--itr)->ProcessEvent(event);
    
      if (event->handled) {
        itr = layer_stack->begin();
      }
    }

    if (!event->handled) {
      if (event->Type() == EventType::SHUTDOWN) {
        ShutdownEvent* sd = Cast<ShutdownEvent>(event);
        if (sd != nullptr) {
          exit_code = sd->GetExitCode();
        }
      }
    }
  }

  void Engine::PushEngineLayer(Ref<Layer>& layer) {
    layer->Attach();
    layer_stack->PushLayer(layer);

    EventQueue::PushEvent<EngineLayerEvent>(LayerEventType::LAYER_PUSH , layer->GetUUID() , layer->Name());
  }

  void Engine::PopEngineLayer(Ref<Layer>& layer) {
    layer->Detach();
    layer_stack->PopLayer(layer);

    EventQueue::PushEvent<EngineLayerEvent>(LayerEventType::LAYER_POP , layer->GetUUID() , layer->Name());
  }

  void Engine::PopEngineLayer() {
    if (layer_stack->Empty()) {
      OE_WARN("Attempting to pop a layer from an empty layer stack");
      return;
    }

    layer_stack->Top()->Detach();
    EventQueue::PushEvent<EngineLayerEvent>(LayerEventType::LAYER_POP , layer_stack->Top()->GetUUID() , layer_stack->Top()->Name());
    layer_stack->PopLayer();
  }

  void Engine::PushEngineOverlay(Ref<Layer>& overlay) {
    overlay->Attach();
    layer_stack->PushOverlay(overlay);

    EventQueue::PushEvent<EngineLayerEvent>(LayerEventType::OVERLAY_PUSH , overlay->GetUUID() , overlay->Name());
  }

  void Engine::PopEngineOverlay(Ref<Layer>& overlay) {
    overlay->Detach();
    layer_stack->PopOverlay(overlay);

    EventQueue::PushEvent<EngineLayerEvent>(LayerEventType::OVERLAY_POP , overlay->GetUUID() , overlay->Name());
  }

} // namespace other

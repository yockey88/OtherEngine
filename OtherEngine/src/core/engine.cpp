/**
 * \file core/engine.cpp
 */
#include "core/engine.hpp"

#include "core/logger.hpp"
#include "event/event.hpp"
#include "event/core_events.hpp"

namespace other {

  Engine::Engine(const ConfigTable& config) 
      : config(config) {
    layer_stack = NewScope<LayerStack>();
    asset_handler = NewScope<AssetHandler>();
    logger_instance = Logger::Instance();
  }

  void Engine::LoadApp(Scope<App>& app) {
    OE_ASSERT(app != nullptr , "Attempting to load a null application");

    active_app = std::move(app);
    active_app->OnLoad();

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
      (*--itr)->OnEvent(event);
    
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
    layer->OnAttach();
    layer_stack->PushLayer(layer);
  }

  void Engine::PushEngineOverlay(Ref<Layer>& overlay) {
    overlay->OnAttach();
    layer_stack->PushOverlay(overlay);
  }

  void Engine::PopEngineLayer(Ref<Layer>& layer) {
    layer->OnDetach();
    layer_stack->PopLayer(layer);
  }

  void Engine::PopEngineOverlay(Ref<Layer>& overlay) {
    overlay->OnDetach();
    layer_stack->PopOverlay(overlay);
  }

} // namespace other

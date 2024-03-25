/**
 * \file core/engine.hpp
 */
#ifndef OTHER_ENGINE_ENGINE_HPP
#define OTHER_ENGINE_ENGINE_HPP

#include "core/defines.hpp"
#include "core/layer_stack.hpp"
#include "event/event_queue.hpp"
#include "application/app.hpp"
#include "asset/asset_handler.hpp"

namespace other {

  class Engine {
    public:
      Opt<ExitCode> exit_code = std::nullopt;

      Engine() = default;
      Engine(const ConfigTable& config);

      void LoadApp(Scope<App>& app);
      Scope<App>& ActiveApp() { return active_app; }
      void UnloadApp();

      void ProcessEvent(Event* event);

      void PushEngineLayer(Ref<Layer>& layer);
      void PushEngineOverlay(Ref<Layer>& overlay);
      void PopEngineLayer(Ref<Layer>& layer);
      void PopEngineOverlay(Ref<Layer>& overlay);

      Logger* GetLog() const { return logger_instance; }

    private:
      ConfigTable config;

      Logger* logger_instance = nullptr;

      Scope<App> active_app;
      Scope<LayerStack> layer_stack;
      Scope<AssetHandler> asset_handler;
  };

} // namespace other

#endif // !OTHER_ENGINE_ENGINE_HPP

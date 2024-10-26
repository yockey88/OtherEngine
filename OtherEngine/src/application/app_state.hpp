/**
 * \file application/app_state.hpp
 **/
#ifndef OTHER_ENGINE_APP_STATE_HPP
#define OTHER_ENGINE_APP_STATE_HPP

#include "core/defines.hpp"
#include "core/layer_stack.hpp"

#include "asset/asset_handler.hpp"
#include "project/project.hpp"

#include "scene/scene_manager.hpp"

#include "rendering/ui/ui_window.hpp"
#include "rendering/ui/ui_window_map.hpp"

namespace other {

  class App;

  class AppState {
   public:
    static void Initialize(const CmdLine& cmd_line, const ConfigTable& config, App* app_handle);
    static void Shutdown();

    static Ref<Project> ProjectContext();
    static Ref<AssetHandler> Assets();

    static Scope<LayerStack>& Layers();
    static Scope<SceneManager>& Scenes();

    static UUID PushUIWindow(Ref<UIWindow> window);
    static void PopUIWindow(UUID id);

    static UUID PushLayer(Ref<Layer> layer);
    static void PopLayer(Opt<UUID> id);

    static App& AppHandle();

    static void AttachApplication();
    static void DetachApplication();

    static void RunEarlyUpdate();
    static void RunUpdate();
    static void RunLateUpdate();
    static void HandleRender();

    inline static EngineMode mode = EngineMode::EDITOR;

   private:
    friend class Engine;
    static void OnEngineTick(float dt);

    static Opt<Path> FindSceneFileByName(const std::string_view name);

    struct Data {
      App* app_handle;  /// do not delete
      CmdLine cmd_line;
      ConfigTable config;

      Scope<LayerStack> layers;
      Scope<SceneManager> scenes;

      Ref<AssetHandler> assets;
      Ref<Project> project;

      UIWindowMap ui_windows;

      float frame_delta = 0.0f;

      Data(App* app_handle, Ref<Project> proj);
      ~Data();
    };

    static Scope<Data> state;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_APP_STATE_HPP

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

#include "engine/engine_state_machine.hpp"

namespace other {

  class App;

  class AppState {
   public:
    static void Initialize(const CmdLine& cmd_line, const ConfigTable& config);
    static void Shutdown();

    static CmdLine& GetProcessArguments();
    static ConfigTable& GetLoadedConfig();

    static Ref<Project> ProjectContext();
    static Ref<AssetHandler> Assets();

    static Scope<LayerStack>& Layers();
    static Scope<SceneManager>& Scenes();

    static Ref<SceneRenderer> GetSceneRenderer();

    static UUID PushUIWindow(Ref<UIWindow> window);
    static void PopUIWindow(UUID id);

    static UUID PushLayer(Ref<Layer> layer);
    static void PopLayer(Opt<UUID> id);

    static App& AppHandle();

    static void AppEvent(const Ref<EngineStateEvent>& event);

    inline static EngineMode mode = EngineMode::EDITOR;
    inline static Opt<ExitCode> exit_code = std::nullopt;
    inline static bool is_attached = false;

    struct Data : public RefCounted {
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

    inline Ref<Data> GetData();

    static bool IsAttached();
    static bool HasPrimaryScene();

    static void LoadPrimaryScene();

    static void AttachApplication();
    static void DetachApplication();
    static void RunEarlyUpdate();
    static void RunUpdate();
    static void RunLateUpdate();
    static void HandleRender();

   private:
    friend class Engine;
    static void OnEngineTick(float dt);

    static Ref<Data> data;

   public:
    template <typename T>
      requires layer_type<T>
    static Ref<T> PushLayer() {
      Ref<T> layer = NewRef<T>(data->app_handle, data->config);
      PushLayer(layer);
      return layer;
    }
  };

}  // namespace other

#endif  // !OTHER_ENGINE_APP_STATE_HPP

/**
 * \file application/app_state.hpp
 **/
#ifndef OTHER_ENGINE_APP_STATE_HPP
#define OTHER_ENGINE_APP_STATE_HPP

#include "core/defines.hpp"
#include "core/layer_stack.hpp"
#include "core/scope.hpp"
#include "engine/engine_states.hpp"

#include "asset/asset_handler.hpp"
#include "project/project.hpp"

#include "scene/scene_manager.hpp"

#include "rendering/ui/ui_window.hpp"
#include "rendering/ui/ui_window_map.hpp"

namespace other {

  class App;
  template <typename T>
  concept AppType = std::derived_from<T, App>;

  class AppState {
   public:
    static void Initialize(Engine* driver);
    static void Shutdown();

    static bool IsLoading();
    static void MarkLoaded();

    static void RebindScripts();

    static CmdLine& GetProcessArguments();
    static ConfigTable& GetLoadedConfig();

    static Ref<Project> ProjectContext();
    static Ref<AssetHandler> Assets();

    static Scope<LayerStack>& Layers();
    static Scope<SceneManager>& Scenes();

    static UUID PushUIWindow(Ref<UIWindow> window);
    static void PopUIWindow(UUID id);

    static UUID PushLayer(Ref<Layer> layer);
    static void PopLayer(Opt<UUID> id);

    static float TargetTimeStep();

    static App& AppHandle();

    /**
     * @note AVOID using this if possible, only use this if you're certain you have to and even then
     *        you should re-evaluate your choices
     */
    template <typename T>
      requires AppType<T>
    static T& AppHandle() {
      OE_ASSERT(data != nullptr, "Can not access app data until app is loaded");
      OE_ASSERT(data->app_handle != nullptr, "Can not access app handle until app is loaded");
      T* app = dynamic_cast<T*>(data->app_handle);
      OE_ASSERT(app != nullptr, "App handle is not of type {}", typeid(T).name());
      return *app;
    }

    static void AppEvent(EngineStateEvent event);

    inline static EngineMode mode = EngineMode::EDITOR;
    inline static Opt<ExitCode> exit_code = std::nullopt;
    inline static bool is_attached = false;

    struct Data : public RefCounted {
      App* app_handle = nullptr;  /// do not delete
      Engine* driver = nullptr;   /// do not delete
      CmdLine cmd_line;
      ConfigTable config;

      Scope<LayerStack> layers;
      Scope<SceneManager> scenes;

      Ref<AssetHandler> assets;
      Ref<Project> project;

      UIWindowMap ui_windows;

      float frame_delta = 0.0f;
      bool loading = true;

      Data(App* app_handle, Ref<Project> proj);
      ~Data();
    };

    inline Ref<Data> GetData();

    static bool IsAttached();
    static bool HasPrimaryScene();

    static void LoadPrimaryScene();

    static void AttachApplication();
    static void DetachApplication();

    static void FlushUpdateLoop();

    static void RunEarlyUpdate();
    static void RunUpdate();
    static void RunLateUpdate();

    static void HandleRender();

   private:
    friend class Engine;
    MOCK_ENGINE_FRIEND;
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

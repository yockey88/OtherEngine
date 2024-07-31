/**
 * \file application\app.hpp
 **/
#ifndef OTHER_ENGINE_APP_HPP
#define OTHER_ENGINE_APP_HPP

#include "core/defines.hpp"
#include "core/config.hpp"
#include "core/layer.hpp"
#include "core/layer_stack.hpp"
#include "project/project.hpp"

#include "asset/asset_handler.hpp"
#include "scene/scene_manager.hpp"

#include "rendering/ui/ui_window.hpp"
#include "rendering/ui/ui_window_map.hpp"

namespace other {

  class Engine;

  class App { 
    public:
      App(Engine* engine);
      virtual ~App();

      Ref<Project> GetProjectContext();
    
      void Load();
      void Run();
      void Unload();

      void PushLayer(Ref<Layer>& layer);
      void PushOverlay(Ref<Layer>& overlay);
      void PopLayer(Ref<Layer>& layer);
      /// pops the last layer pushed 
      void PopLayer();
      void PopOverlay(Ref<Layer>& overlay);
    
      void ProcessEvent(Event* event);

      Ref<UIWindow>& GetUIWindow(const std::string& name);
      Ref<UIWindow>& GetUIWindow(UUID id);
      UUID PushUIWindow(const std::string& name , Ref<UIWindow> window);
      UUID PushUIWindow(Ref<UIWindow> window);
      bool RemoveUIWindow(const std::string& name);
      bool RemoveUIWindow(UUID id);

      void LoadScene(const Path& path);
      bool HasActiveScene();
      SceneMetadata* ActiveScene();
      void UnloadScene();

      void ReloadScripts();
      
      Ref<Project> project_metadata;
      Ref<AssetHandler> asset_handler = nullptr;

      Scope<LayerStack> layer_stack = nullptr;
      Scope<SceneManager> scene_manager = nullptr;

    protected:
      void Attach();
      /// this is seperate because this triggers events which need to be polled while update
      ///  responds to process events
      void DoEarlyUpdate(float dt);
      void DoUpdate(float dt);
      void DoRender();
      void DoRenderUI();
      void Detach();

      virtual void OnLoad() {}
      virtual void OnAttach() {} 

      virtual void OnEvent(Event* event) {}
      virtual void EarlyUpdate(float dt) {}
      virtual void Update(float dt) {}
      virtual void Render() {}
      virtual void RenderUI() {}
      virtual void LateUpdate(float dt) {}

      virtual void OnDetach() {}
      virtual void OnUnload() {}

      virtual void OnSceneLoad(const SceneMetadata* path) {}
      virtual void OnSceneUnload() {}

      /// will only ever be called if editor is active because otherwise the scripts
      ///   wont be reloaded
      virtual void OnScriptReload() {}

      virtual Ref<AssetHandler> CreateAssetHandler();

      Engine* engine_handle = nullptr;

      const CmdLine& cmdline;
      const ConfigTable& config;

      UIWindowMap ui_windows;

      bool lost_window_focus = false;

      enum LayerFlags : uint64_t {
        NO_LAYER_FLAGS = 0 ,
        
        LAYER_PUSHED_EU = bit(1) ,
        LAYER_PUSHED_U = bit(2) , 
        LAYER_PUSHED_LU = bit(3) , 

        LAYER_PUSHED_R = bit(4) ,
        LAYER_PUSHED_RUI = bit(5) , 
      };

      friend class Engine;
      friend class AppState;
      friend class Editor;

    private:
      Engine* GetEngine() const { return engine_handle; }
  };

} // namespace other

#endif // !OTHER_ENGINE_APP_HPP

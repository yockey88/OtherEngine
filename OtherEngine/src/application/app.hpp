/**
 * \file application\app.hpp
 **/
#ifndef OTHER_ENGINE_APP_HPP
#define OTHER_ENGINE_APP_HPP

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

    protected:
      bool InEditor() const;
      
      void Attach();
      void DoEarlyUpdate(float dt);
      void DoUpdate(float dt);
      void DoRender();
      void DoRenderUI();
      void DoLateUpdate(float dt);
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

      Engine* engine_handle = nullptr;

      const CmdLine& cmdline;
      const ConfigTable& config;

      Ref<Project> project_metadata;
      Ref<AssetHandler> asset_handler = nullptr;

      Scope<LayerStack> layer_stack = nullptr;
      Scope<SceneManager> scene_manager = nullptr;

      UIWindowMap ui_windows;

      bool is_editor = false;
      bool in_editor = false;
      bool lost_window_focus = false;

      friend class Engine;
      friend class AppState;
      friend class Editor;

      void SetInEditor();

      void ReloadScripts();

    private:
      Engine* GetEngine() const { return engine_handle; }
  };

} // namespace other

#endif // !OTHER_ENGINE_APP_HPP

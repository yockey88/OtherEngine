/**
 * \file application\app.hpp
 **/
#ifndef OTHER_ENGINE_APP_HPP
#define OTHER_ENGINE_APP_HPP

#include "core/config.hpp"
#include "core/layer.hpp"
#include "core/layer_stack.hpp"
#include "project/project.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "asset/asset_handler.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/ui/ui_window.hpp"
#include "scene/scene.hpp"
#include "scene/scene_manager.hpp"

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
      const SceneMetadata* ActiveScene();
      void UnloadScene();

    protected:

      bool InEditor() const;
      
      void Attach();
      void DoUpdate(float dt);
      void DoRender();
      void DoRenderUI();
      void Detach();

      virtual void OnLoad() {}
      virtual void OnAttach() {} 
      virtual void OnEvent(Event* event) {}
      virtual void Update(float dt) {}
      virtual void Render() {}
      virtual void RenderUI() {}
      virtual void OnDetach() {}
      virtual void OnUnload() {}

      virtual void OnSceneLoad(const SceneMetadata* path) {}
      virtual void OnSceneUnload() {}

      /// will only ever be called if editor is active because otherwise the scripts
      ///   wont be reloaded
      virtual void OnScriptReload() {}

      Engine* GetEngine() { return engine_handle; }

      const CmdLine& cmdline;
      const ConfigTable& config;

    private:
      Ref<Project> project_metadata;

      Scope<LayerStack> layer_stack = nullptr;
      Scope<AssetHandler> asset_handler = nullptr;
      Scope<SceneManager> scene_manager = nullptr;

      std::map<UUID , Ref<UIWindow>> ui_windows;

      Engine* engine_handle = nullptr;

      bool is_editor = false;
      bool in_editor = false;
      bool lost_window_focus = false;

      friend class Editor;
      void SetInEditor();

      void ReloadScripts();
  };

} // namespace other

#endif // !OTHER_ENGINE_APP_HPP

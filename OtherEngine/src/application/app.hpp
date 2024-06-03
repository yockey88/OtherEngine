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
#include "rendering/ui/ui_window.hpp"
#include "scene/scene.hpp"
#include "scene/scene_manager.hpp"

namespace other {

  class Engine;

  class App { 
    public:
      App(Engine* engine);
      virtual ~App();

      void LoadMetadata(const Ref<Project>& project);
    
      void OnLoad();
      void Run();
      void OnUnload();

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
      Ref<Scene> ActiveScene();
      void UnloadScene();

    protected:
      
      void Attach();
      void DoUpdate(float dt);
      void DoRender();
      void DoRenderUI();
      void Detach();

      virtual void OnAttach() = 0; 
      virtual void OnEvent(Event* event) = 0;
      virtual void Update(float dt) = 0;
      virtual void Render() = 0;
      virtual void RenderUI() = 0;
      virtual void OnDetach() = 0;

      virtual void OnSceneLoad(const SceneMetadata* path) {}

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

      friend class Editor;
  };

} // namespace other

#endif // !OTHER_ENGINE_APP_HPP

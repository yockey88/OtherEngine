/**
 * \file application\app.hpp
 **/
#ifndef OTHER_ENGINE_APP_HPP
#define OTHER_ENGINE_APP_HPP

#include "core/config.hpp"
#include "core/layer.hpp"
#include "core/layer_stack.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "asset/asset_handler.hpp"
#include "rendering/ui/ui_window.hpp"

namespace other {

  class Engine;

  class App { 
    public:
      App(Engine* engine);
      virtual ~App();
    
      void OnLoad();
      void Attach();
      void Run();

      void DoUpdate(float dt);
      void DoRender();
      void DoRenderUI();

      void OnUnload();
      void Detach();

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

    protected:

      virtual void OnAttach() = 0; 
      virtual void OnEvent(Event* event) = 0;
      virtual void Update(float dt) = 0;
      virtual void Render() = 0;
      virtual void RenderUI() = 0;
      virtual void OnDetach() = 0;

      Engine* GetEngine() { return engine_handle; }

      const CmdLine& cmdline;
      const ConfigTable& config;
    private:
      Scope<LayerStack> layer_stack = nullptr;
      Scope<AssetHandler> asset_handler = nullptr;

      std::map<UUID , Ref<UIWindow>> ui_windows;

      Engine* engine_handle = nullptr;
  };

} // namespace other

#endif // !OTHER_ENGINE_APP_HPP

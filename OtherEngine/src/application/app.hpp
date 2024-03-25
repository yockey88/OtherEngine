/**
 * \file application\app.hpp
 **/
#ifndef OTHER_ENGINE_APP_HPP
#define OTHER_ENGINE_APP_HPP

#include <string>

#include "core/config.hpp"
#include "core/layer.hpp"
#include "core/layer_stack.hpp"
#include "asset/asset_handler.hpp"

namespace other {

  class Engine;

  class App { 
    public:
      App(Engine* engine , const ConfigTable& cfg);
      virtual ~App();
    
      void OnLoad();
      void Run();
      void OnUnload();

      void PushLayer(Ref<Layer>& layer);
      void PushOverlay(Ref<Layer>& overlay);
      void PopLayer(Ref<Layer>& layer);
      void PopOverlay(Ref<Layer>& overlay);
    
      void ProcessEvent(Event* event);

    protected:
      virtual void OnAttach() = 0; 
      virtual void OnEvent(Event* event) = 0;
      virtual void Update(float dt) = 0;
      virtual void Render() = 0;
      virtual void RenderUI() = 0;
      virtual void OnDetach() = 0;

      Engine* GetEngine() { return engine_handle; }

      void InitUIContext();
    
      void LoadScene(const std::string& scene_file);
    
    private:
      Scope<LayerStack> layer_stack = nullptr;
      Scope<AssetHandler> asset_handler = nullptr;

      Engine* engine_handle = nullptr;
      ConfigTable config;
  };

} // namespace other

#endif // !OTHER_ENGINE_APP_HPP

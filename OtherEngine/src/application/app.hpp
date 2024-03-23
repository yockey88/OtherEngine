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

  struct AppData {
    // Scope<AppLoader> app_loader = nullptr;
    // ProjectData project_data;
    // AppConfig app_config;
    // AppVersion app_version;
    std::string author;
    std::string project_folder;
    std::string project_file;
    std::string project_lib;
  };

  class App { 
    public:
      App(Engine* engine , const ConfigTable& cfg);
      virtual ~App();
    
      void OnLoad();
      void Run();
      void OnUnload();

      void PushLayer(Scope<Layer>& layer);
      void PushOverlay(Scope<Layer>& overlay);
      void PopLayer(Scope<Layer>& layer);
      void PopOverlay(Scope<Layer>& overlay);
    
      void ProcessEvent(Event* event);

      virtual void OnEvent(Event* event) {}
      virtual void Update(float dt) {}
      virtual void Render() {}
      virtual void RenderUI() {}
    
      inline AppData *GetAppData() { return app_data; }
      // inline const AppConfig &GetConfig() const { return app_data->app_config; }

    protected:
      // Scope<Scene> scene_context = nullptr;

      Engine* GetEngine() { return engine_handle; }

      void InitUIContext();
    
      void LoadScene(const std::string& scene_file);
    
      virtual void Initialize() {}
      virtual void Shutdown() {}
    
    private:
      AppData *app_data = nullptr;
      Scope<LayerStack> layer_stack = nullptr;
      Scope<AssetHandler> asset_handler = nullptr;

      Engine* engine_handle = nullptr;
      ConfigTable config;
  };

  Scope<App> CreateApp(Engine* engine , const ConfigTable& config);

} // namespace other

#endif // !OTHER_ENGINE_APP_HPP

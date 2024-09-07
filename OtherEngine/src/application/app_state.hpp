/**
 * \file application/app_state.hpp
 **/
#ifndef OTHER_ENGINE_APP_STATE_HPP
#define OTHER_ENGINE_APP_STATE_HPP

#include "core/layer_stack.hpp"
#include "asset/asset_handler.hpp"
#include "scene/scene_manager.hpp"
#include "project/project.hpp"
#include "rendering/ui/ui_window.hpp"

namespace other {

  class App;

  class AppState {
    public:
      static void Initialize(App* app_handle , Scope<LayerStack>& layers , Scope<SceneManager>& scenes , 
                             Ref<AssetHandler>& assets , Ref<Project>& project_contex);

      static Ref<Project> ProjectContext();
      static Ref<AssetHandler> Assets();

      static Scope<LayerStack>& Layers();
      static Scope<SceneManager>& Scenes();

      static UUID PushUIWindow(Ref<UIWindow> window);

      inline static EngineMode mode = EngineMode::DEBUG;

    private:
      struct Data {
        App* app_handle; /// do not delete
                         
        Scope<LayerStack>& layers;
        Scope<SceneManager>& scenes;

        Ref<AssetHandler> assets;
        Ref<Project> project;

        Data(App* app_handle , Scope<LayerStack>& layers , Scope<SceneManager>& scenes , Ref<AssetHandler>& assets , Ref<Project>& context)
            : app_handle(app_handle) , layers(layers) , scenes(scenes) , assets(assets) , project(context) {}
      };

      static Scope<Data> state;
  };

} // namespace other

#endif // !OTHER_ENGINE_APP_STATE_HPP


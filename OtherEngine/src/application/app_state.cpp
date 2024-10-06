/**
 * \file application/app_state.cpp
 **/
#include "application/app_state.hpp"

#include "core/logger.hpp"
#include "application/app.hpp"

namespace other {

  Scope<AppState::Data> AppState::state = nullptr;

  void AppState::Initialize(App* app_handle , Scope<LayerStack>& layers , Scope<SceneManager>& scenes, 
                             Ref<AssetHandler>& assets , Ref<Project>& context) {
    OE_ASSERT(app_handle != nullptr , "Can not load null application");
    OE_ASSERT(layers != nullptr , "Can not load null layer stack into app state");
    OE_ASSERT(scenes != nullptr , "Can not load null scene manager into app state");
    OE_ASSERT(assets != nullptr , "Can not load null asset handler into app state");
    OE_ASSERT(context != nullptr , "Can not load null project context into app state");
    state = NewScope<Data>(app_handle , layers , scenes , assets , context); 
  }

  void AppState::Shutdown() {
    state = nullptr;
  }

  Ref<Project> AppState::ProjectContext() {
    OE_ASSERT(state != nullptr , "Can not access app state until app is loaded");
    return Ref<Project>::Clone(state->project);

  }
  
  Ref<AssetHandler> AppState::Assets() {
    OE_ASSERT(state != nullptr , "Can not access app state until app is loaded");
    return Ref<AssetHandler>::Clone(state->assets);
  }
      
  Scope<LayerStack>& AppState::Layers() {
    OE_ASSERT(state != nullptr , "Can not access app state until app is loaded");
    return state->layers;
  }

  Scope<SceneManager>& AppState::Scenes() {
    OE_ASSERT(state != nullptr , "Can not access app state until app is loaded");
    return state->scenes;
  }
      
  UUID AppState::PushUIWindow(Ref<UIWindow> window) {
    OE_ASSERT(state != nullptr , "Can not access app state until app is loaded");
    OE_ASSERT(state->app_handle != nullptr , "Can not access app state until app is loaded");
    return state->app_handle->PushUIWindow(window);
  }

} // namespace other

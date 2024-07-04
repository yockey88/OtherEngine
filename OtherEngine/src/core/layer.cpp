/**
 * \file core/layer.cpp
 */
#include "core/layer.hpp"

namespace other {

  void Layer::Attach() {
    OnAttach();
  }
      
  void Layer::EarlyUpdate(float dt) {
    OnEarlyUpdate(dt);
  }

  void Layer::Update(float dt) {
    OnUpdate(dt);
  }
  
  void Layer::LateUpdate(float dt) {
    OnLateUpdate(dt);
  }

  void Layer::Render() {
    OnRender();
  }

  void Layer::UIRender() {
    OnUIRender();
  }

  void Layer::ProcessEvent(Event* event) {
    OnEvent(event);
  }

  void Layer::Detach() {
    OnDetach();
  }
      
  void Layer::LoadScene(const SceneMetadata* metadata) {
    OnSceneLoad(metadata);
  }

  void Layer::UnloadScene() {
    OnSceneUnload();
  }

  void Layer::ReloadScripts() {
    OnScriptReload();
  }

} // namespace other

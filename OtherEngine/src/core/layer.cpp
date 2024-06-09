/**
 * \file core/layer.cpp
 */
#include "core/layer.hpp"

namespace other {

  void Layer::Attach() {
    OnAttach();
  }

  void Layer::Update(float dt) {
    OnUpdate(dt);
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

} // namespace other

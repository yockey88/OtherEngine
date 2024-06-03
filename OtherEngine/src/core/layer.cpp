/**
 * \file core/layer.cpp
 */
#include "core/layer.hpp"

#include "core/logger.hpp"

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
    OE_ASSERT(metadata != nullptr , "Attempting to pass null scene metadata into layer");
    OnSceneLoad(metadata);
  }

} // namespace other

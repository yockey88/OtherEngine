/**
 * \file application/runtime_layer.cpp
 **/
#include "application/runtime_layer.hpp"

#include "application/app_state.hpp"

namespace other {

  void RuntimeLayer::OnAttach() {
    AppState::Scenes()->StartScene();
  }

  void RuntimeLayer::OnDetach() {
    AppState::Scenes()->StopScene();
  }

  void RuntimeLayer::OnEarlyUpdate(float dt) {
  }

  void RuntimeLayer::OnUpdate(float dt) {
  }

  void RuntimeLayer::OnLateUpdate(float dt) {
  }

  void RuntimeLayer::OnRender() {}

  void RuntimeLayer::OnUIRender() {
    AppState::Scenes()->RenderSceneUI();
  }

}  // namespace other

/**
 * \file application/runtime_layer.cpp
 **/
#include "application/runtime_layer.hpp"

#include "core/config_keys.hpp"
#include "application/app.hpp"
#include "application/app_state.hpp"
#include "layers/debug_layer.hpp"

namespace other {

  void RuntimeLayer::OnAttach() {
    auto debug = config.GetVal<bool>(kProjectSection , kDebugValue);
    if (debug.has_value() && debug.value()) {
      OE_DEBUG("Pushing debug layer");

      Ref<Layer> debug_layer = NewRef<DebugLayer>(ParentApp());
      ParentApp()->PushLayer(debug_layer);
    }

    environment = NewRef<Environment>();
  }

  void RuntimeLayer::OnEarlyUpdate(float dt) {
    AppState::Scenes()->EarlyUpdateScene(dt);
  }

  void RuntimeLayer::OnUpdate(float dt) {
    AppState::Scenes()->UpdateScene(dt);
  }

  void RuntimeLayer::OnLateUpdate(float dt) {
    AppState::Scenes()->LateUpdateScene(dt);
  }

  void RuntimeLayer::OnRender() {
    AppState::Scenes()->RenderScene(scene_renderer);
  }

  void RuntimeLayer::OnUIRender() {
    AppState::Scenes()->RenderSceneUI();
  }

} // namespace other

/**
 * \file rendering_layer.cpp
 **/
#include "rendering_layer.hpp"

#include "application/app_state.hpp"
#include "event/event_queue.hpp"
#include "input/mouse.hpp"

#include "rendering/perspective_camera.hpp"
#include "rendering/renderer.hpp"

using namespace other;

void RenderingLayer::OnAttach() {
  EventQueue::RegisterEventDispatcher<KeyPressed>(
    "Rendering-Layer-Key-Bindings",
    { std::bind_front(&RenderingLayer::HandleKeyPressed, this) }
  );

  auto win_size = Renderer::WindowSize();
  camera = NewRef<PerspectiveCamera>(glm::ivec2{ win_size.x, win_size.y });
  camera->SetPosition({ 0.f, 0.f, 3.f });
  DefaultUpdateCamera(camera);

  Ref<Shader> fbshader = BuildShader(fbshader_path);
}

void RenderingLayer::OnDetach() {}

void RenderingLayer::OnLateUpdate(float dt) {
  if (camera == nullptr) {
    return;
  }

  if (!camera_lock && AppState::mode != EngineMode::EDITOR) {
    DefaultUpdateCamera(camera);
  }
}

void RenderingLayer::OnRender() {
  if (AppState::mode != EngineMode::EDITOR) {
    AppState::Scenes()->GetRenderer()->SubmitCamera(camera);
  }
}

void RenderingLayer::OnUIRender() {}

bool RenderingLayer::HandleKeyPressed(KeyPressed& event) {
  HandleKeyEvent(event, Keyboard::Key::OE_C, [&]() {
    camera_lock = !camera_lock;
    if (camera_lock) {
      Mouse::FreeCursor();
    } else {
      Mouse::LockCursor();
    }
  });

  return event.Key() != Keyboard::Key::OE_C;
}
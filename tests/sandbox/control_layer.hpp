/**
 * \file control_layer.hpp
 **/
#ifndef OTHER_ENGINE_CONTROL_LAYER_HPP
#define OTHER_ENGINE_CONTROL_LAYER_HPP

#include "core/layer.hpp"

#include "application/app.hpp"
#include "event/key_events.hpp"
#include "event/window_events.hpp"

using namespace other;

class ControlLayer : public Layer {
 public:
  ControlLayer(App* parent_app, const std::string& name)
      : Layer(parent_app, name) {}

 protected:
  virtual void OnAttach() override;
  // virtual void OnDetach() {}
  // virtual void OnEarlyUpdate(float dt) {}
  // virtual void OnUpdate(float dt) {}
  // virtual void OnLateUpdate(float dt) {}
  // virtual void OnRender() {}
  // virtual void OnUIRender() {}

  bool HandleWindowClosed(WindowClosed& event);
  bool HandleKeyPress(KeyPressed& event);
};

#endif  // !OTHER_ENGINE_CONTROL_LAYER_HPP
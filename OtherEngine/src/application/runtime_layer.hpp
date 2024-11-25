/**
 * \file application/runtime_layer.hpp
 **/
#ifndef OTHER_ENGINE_RUNTIME_LAYER_HPP
#define OTHER_ENGINE_RUNTIME_LAYER_HPP

#include "core/layer.hpp"

#include "scene/light_environment.hpp"

#include "rendering/scene_renderer.hpp"

namespace other {

  class RuntimeLayer : public Layer {
   public:
    RuntimeLayer(App* app, const ConfigTable& config)
        : Layer(app, "RuntimeLayer"), config(config) {}

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnEarlyUpdate(float dt) override;
    virtual void OnUpdate(float dt) override;
    virtual void OnLateUpdate(float dt) override;
    virtual void OnRender() override;
    virtual void OnUIRender() override;

   private:
    const ConfigTable& config;

    Ref<SceneRenderer> scene_renderer = nullptr;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_RUNTIME_LAYER_HPP

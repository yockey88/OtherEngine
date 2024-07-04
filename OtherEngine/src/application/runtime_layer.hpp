/**
 * \file application/runtime_layer.hpp
 **/
#ifndef OTHER_ENGINE_RUNTIME_LAYER_HPP
#define OTHER_ENGINE_RUNTIME_LAYER_HPP

#include "core/layer.hpp"

namespace other {

  class RuntimeLayer : public Layer {
    public:
      RuntimeLayer(App* app , const ConfigTable& config)
        : Layer(app , "RuntimeLayer") , config(config) {}

      virtual void OnAttach() override;
      virtual void OnDetach() override {}
      virtual void OnEarlyUpdate(float dt) override;
      virtual void OnUpdate(float dt) override;
      virtual void OnLateUpdate(float dt) override;
      virtual void OnRender() override;
      virtual void OnUIRender() override;
      virtual void OnEvent(Event* event) override {}

      virtual void OnSceneLoad(const SceneMetadata* metadata) override {}
      virtual void OnSceneUnload() override {}

      virtual void OnScriptReload() override {}

    private:
      const ConfigTable& config;
  }; 

} // namespace other

#endif // !OTHER_ENGINE_RUNTIME_LAYER_HPP

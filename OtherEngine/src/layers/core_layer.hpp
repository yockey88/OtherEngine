/**
 * \file layers/core_layer.hpp
 **/
#ifndef OTHER_ENGINE_CORE_LAYER_HPP
#define OTHER_ENGINE_CORE_LAYER_HPP

#include "core/layer.hpp"
#include "core/ref.hpp"
#include "rendering/ui/ui_window.hpp"

namespace other {

  class CoreLayer : public Layer {
    public:
      CoreLayer(App* parent_app)
        : Layer(parent_app , "CoreLayer") {}
      virtual ~CoreLayer() override {}

      virtual void OnAttach() override;
      virtual void OnUpdate(float dt) override;
      virtual void OnRender() override;
      virtual void OnUIRender() override;
      virtual void OnEvent(Event* event) override;
  };

} // namespace other

#endif // !OTHER_ENGINE_CORE_LAYER_HPP

/**
 * \file layers/debug_layer.hpp
 **/
#ifndef OTHER_ENGINE_DEBUG_LAYER_HPP
#define OTHER_ENGINE_DEBUG_LAYER_HPP

#include "core/layer.hpp"

namespace other {

  class DebugLayer : public Layer {
    public:
      DebugLayer(App* parent_app)
        : Layer(parent_app , "DebugLayer") {}
      virtual ~DebugLayer() override {}

      virtual void OnAttach() override;
      virtual void OnUpdate(float dt) override;
      virtual void OnRender() override;
      virtual void OnUIRender() override;
      virtual void OnDetach() override;

    private:
      float last_frame_time = 0.0f;

      constexpr static size_t kFpsSamples = 100;
      float fps_data[kFpsSamples] = {0.0f};
      size_t fps_data_index = 0;
  };

} // namespace other
  
#endif // !OTHER_ENGINE_DEBUG_LAYER_HPP

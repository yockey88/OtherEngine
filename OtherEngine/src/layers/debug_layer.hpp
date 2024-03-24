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
  };

} // namespace other
  
#endif // !OTHER_ENGINE_DEBUG_LAYER_HPP

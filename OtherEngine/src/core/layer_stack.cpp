/**
 * \file core/layer_stack.cpp
 */
#include "core/layer_stack.hpp"

#include "core/logger.hpp"

namespace other {

  LayerStack::~LayerStack() {
    for (auto& layer : layers) {
      layer->OnDetach();
    }
  }

  void LayerStack::PushLayer(Scope<Layer>& layer) {
    layers.emplace(layers.begin() + layer_insert_index, std::move(layer));
    ++layer_insert_index;
  }

  void LayerStack::PopLayer(Scope<Layer>& layer) {
    auto itr = std::find(layers.begin(), layers.end(), layer);
    if (itr != layers.end()) {
      layers.erase(itr);
      --layer_insert_index;
    } else {
      OE_WARN("Attempting to pop a layer that does not exist");
    }
  }

  void LayerStack::PushOverlay(Scope<Layer>& overlay) {
    layers.emplace_back(std::move(overlay));
  }

  void LayerStack::PopOverlay(Scope<Layer>& overlay) {
    auto itr = std::find(layers.begin(), layers.end(), overlay);
    if (itr != layers.end()) {
      layers.erase(itr);
    } else {
      OE_WARN("Attempting to pop an overlay that does not exist");
    }
  }

  Scope<Layer>& LayerStack::operator[](size_t index) {
    OE_ASSERT(index < layers.size() , "Attempting to access a layer that does not exist");
    return layers[index];
  }

  const Scope<Layer>& LayerStack::operator[](size_t index) const {
    OE_ASSERT(index < layers.size() , "Attempting to access a layer that does not exist");
    return layers[index];
  }

  Scope<Layer>& LayerStack::At(size_t index) {
    OE_ASSERT(index < layers.size() , "Attempting to access a layer that does not exist");
    return layers[index];
  }

  const Scope<Layer>& LayerStack::At(size_t index) const {
    OE_ASSERT(index < layers.size() , "Attempting to access a layer that does not exist");
    return layers[index];
  }

  size_t LayerStack::Size() const {
    return layers.size();
  }

  std::vector<Scope<Layer>>::iterator LayerStack::begin() {
    return layers.begin();
  }

  std::vector<Scope<Layer>>::iterator LayerStack::end() {
    return layers.end();
  }

} // namespace other

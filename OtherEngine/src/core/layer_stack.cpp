/**
 * \file core/layer_stack.cpp
 */
#include "core/layer_stack.hpp"

#include "core/logger.hpp"

namespace other {

  LayerStack::~LayerStack() {
    for (auto& layer : layers) {
      layer->Detach();
    }
  }

  void LayerStack::PushLayer(const Ref<Layer>& layer) {
    layers.emplace(layers.begin() + layer_insert_index, layer);
    ++layer_insert_index;
  }

  void LayerStack::PopLayer(const Ref<Layer>& layer) {
    auto itr = std::find(layers.begin(), layers.end(), layer);
    if (itr != layers.end()) {
      layers.erase(itr);
      --layer_insert_index;
    } else {
      OE_WARN("Attempting to pop a layer that does not exist");
    }
  }

  void LayerStack::PopLayer() {
    if (layers.empty()) {
      OE_WARN("Attempting to pop a layer from an empty layer stack");
      return;
    }

    layers.pop_back();
    --layer_insert_index;
  }

  void LayerStack::PushOverlay(const Ref<Layer>& overlay) {
    layers.emplace_back(overlay);
  }

  void LayerStack::PopOverlay(const Ref<Layer>& overlay) {
    auto itr = std::find(layers.begin(), layers.end(), overlay);
    if (itr != layers.end()) {
      layers.erase(itr);
    } else {
      OE_WARN("Attempting to pop an overlay that does not exist");
    }
  }

  Ref<Layer>& LayerStack::operator[](size_t index) {
    OE_ASSERT(index < layers.size() , "Attempting to access a layer that does not exist");
    return layers[index];
  }

  const Ref<Layer>& LayerStack::operator[](size_t index) const {
    OE_ASSERT(index < layers.size() , "Attempting to access a layer that does not exist");
    return layers[index];
  }

  Ref<Layer>& LayerStack::At(size_t index) {
    OE_ASSERT(index < layers.size() , "Attempting to access a layer that does not exist");
    return layers[index];
  }

  const Ref<Layer>& LayerStack::At(size_t index) const {
    OE_ASSERT(index < layers.size() , "Attempting to access a layer that does not exist");
    return layers[index];
  }

  size_t LayerStack::Size() const {
    return layers.size();
  }

  bool LayerStack::Empty() const {
    return layers.empty();
  }

  void LayerStack::Clear() {
    for (auto& layer : layers) {
      layer->Detach();
    }
    layers.clear();
    layer_insert_index = 0;
  }

  Ref<Layer>& LayerStack::Bottom() {
    return layers.front();
  }

  Ref<Layer>& LayerStack::Top() {
    return layers.back();
  }

  std::vector<Ref<Layer>>::iterator LayerStack::begin() {
    return layers.begin();
  }

  std::vector<Ref<Layer>>::iterator LayerStack::end() {
    return layers.end();
  }

} // namespace other

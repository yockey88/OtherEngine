/**
 * \file core/layer_stack.cpp
 */
#include "core/layer_stack.hpp"

#include "core/logger.hpp"

#include "event/app_events.hpp"
#include "event/event_queue.hpp"

namespace other {

  LayerStack::~LayerStack() {
    for (auto& layer : layers) {
      layer->Detach();
    }
  }

  void LayerStack::PushLayer(Ref<Layer>& layer) {
    OE_ASSERT(layer != nullptr, "Attempting to push a null layer");
    OE_ASSERT(layer->GetUUID().Get() != 0, "Layer UUID is 0, this is not allowed");

    layer->Attach();
    layers.emplace(layers.begin() + layer_insert_index, layer);
    ++layer_insert_index;

    EventQueue::PushEvent<AppLayerEvent>({ LayerEventType::LAYER_PUSH, layer->GetUUID().Get() });
  }

  void LayerStack::PopLayer(UUID id) {
    OE_ASSERT(id.Get() != 0, "Attempting to pop a layer with a null UUID");

    for (auto& l : layers) {
      if (l->GetUUID() == id) {
        l->Detach();
        break;
      }
    }

    auto layers_after_removal = layers |
      std::views::filter([id](const Ref<Layer>& l) { return l->GetUUID() != id; }) |
      std::ranges::to<std::vector<Ref<Layer>>>();

    if (layers_after_removal.size() == layers.size()) {
      OE_WARN("Attempting to pop a layer that does not exist");
      return;
    }

    layers.swap(layers_after_removal);
    --layer_insert_index;

    EventQueue::PushEvent<AppLayerEvent>({ LayerEventType::LAYER_POP, id.Get() });
  }

  void LayerStack::PopLayer() {
    if (layers.empty()) {
      OE_WARN("Attempting to pop a layer from an empty layer stack");
      return;
    }

    Ref<Layer>& layer = layers.back();
    OE_ASSERT(layer != nullptr, "Attempting to pop a null layer");

    UUID id = layer->GetUUID();
    OE_ASSERT(id.Get() != 0, "Layer UUID is 0, this is not allowed");
    layers.pop_back();
    --layer_insert_index;

    EventQueue::PushEvent<AppLayerEvent>({ LayerEventType::LAYER_POP, id.Get() });
  }

  Ref<Layer>& LayerStack::operator[](size_t index) {
    OE_ASSERT(index < layers.size(), "Attempting to access a layer that does not exist");
    return layers[index];
  }

  const Ref<Layer>& LayerStack::operator[](size_t index) const {
    OE_ASSERT(index < layers.size(), "Attempting to access a layer that does not exist");
    return layers[index];
  }

  Ref<Layer>& LayerStack::At(size_t index) {
    OE_ASSERT(index < layers.size(), "Attempting to access a layer that does not exist");
    return layers[index];
  }

  const Ref<Layer>& LayerStack::At(size_t index) const {
    OE_ASSERT(index < layers.size(), "Attempting to access a layer that does not exist");
    return layers[index];
  }

  size_t LayerStack::Size() const {
    return layers.size();
  }

  bool LayerStack::Empty() const {
    return layers.empty();
  }

  void LayerStack::Clear() {
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

}  // namespace other

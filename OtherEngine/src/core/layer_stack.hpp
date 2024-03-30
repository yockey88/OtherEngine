/**
 * \file core\layer_stack.hpp
 */
#ifndef OTHER_ENGINE_LAYER_STACK_HPP
#define OTHER_ENGINE_LAYER_STACK_HPP

#include <vector>

#include "core/layer.hpp"
#include "core/ref.hpp"

namespace other {

  class LayerStack {
    public:
      LayerStack() {}
      ~LayerStack(); 

      void PushLayer(const Ref<Layer>& layer);
      void PopLayer(const Ref<Layer>& layer);
      void PopLayer();

      void PushOverlay(const Ref<Layer>& overlay);
      void PopOverlay(const Ref<Layer>& overlay);

      Ref<Layer>& operator[](size_t index);
      const Ref<Layer>& operator[](size_t index) const;
      Ref<Layer>& At(size_t index);
      const Ref<Layer>& At(size_t index) const;

      size_t Size() const;
      bool Empty() const;
      void Clear();

      Ref<Layer>& Bottom();
      Ref<Layer>& Top();

      std::vector<Ref<Layer>>::iterator begin();
      std::vector<Ref<Layer>>::iterator end();

    private:
      std::vector<Ref<Layer>> layers;
      size_t layer_insert_index = 0;

  };

} // namespace other

#endif // !OTHER_ENGINE_LAYER_STACK_HPP

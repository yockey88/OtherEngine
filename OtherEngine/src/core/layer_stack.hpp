/**
 * \file core\layer_stack.hpp
 */
#ifndef OTHER_ENGINE_LAYER_STACK_HPP
#define OTHER_ENGINE_LAYER_STACK_HPP

#include <vector>

#include "core/defines.hpp"
#include "core/layer.hpp"

namespace other {

  class LayerStack {
    public:
      LayerStack() {}
      ~LayerStack(); 

      void PushLayer(Scope<Layer>& layer);
      void PopLayer(Scope<Layer>& layer);
      void PushOverlay(Scope<Layer>& overlay);
      void PopOverlay(Scope<Layer>& overlay);

      Scope<Layer>& operator[](size_t index);
      const Scope<Layer>& operator[](size_t index) const;
      Scope<Layer>& At(size_t index);
      const Scope<Layer>& At(size_t index) const;

      size_t Size() const;

      std::vector<Scope<Layer>>::iterator begin();
      std::vector<Scope<Layer>>::iterator end();

    private:
      std::vector<Scope<Layer>> layers;
      size_t layer_insert_index = 0;

  };

} // namespace other

#endif // !OTHER_ENGINE_LAYER_STACK_HPP

/**
 * \file core\layer.hpp
 */
#ifndef OTHER_ENGINE_LAYER_HPP
#define OTHER_ENGINE_LAYER_HPP

#include <string>

#include "event/event.hpp"

namespace other {

  class App;

  class Layer {
    App* parent_app = nullptr;

    protected:
      std::string debug_name;

      App* GetApp() const { return parent_app; }

    public:
      Layer(App* parent_app , const std::string& name = "Layer")
        : parent_app(parent_app) , debug_name(name) {}
      virtual ~Layer() {}

      virtual void OnAttach() {}
      virtual void OnDetach() {}
      virtual void OnUpdate(float dt) {}
      virtual void OnRender() {}
      virtual void OnUIRender() {}
      virtual void OnEvent(Event* event) {}

      inline const std::string& Name() const { return debug_name; }
  };

} // namespace other

#endif // !OTHER_ENGINE_LAYER_HPP

/**
 * \file core\layer.hpp
 */
#ifndef OTHER_ENGINE_LAYER_HPP
#define OTHER_ENGINE_LAYER_HPP

#include <string>

#include "core/ref_counted.hpp"
#include "core/uuid.hpp"

#include "scene/scene_manager.hpp"

namespace other {

  class App;

  class Layer : public RefCounted {
   public:
    Layer(App* parent_app, const std::string& name)
        : debug_name(name), uuid(FNV(name)), parent_app(parent_app) {}
    virtual ~Layer() {}

    inline const std::string& Name() const { return debug_name; }
    inline const UUID GetUUID() const { return uuid; }

    void Attach();
    void EarlyUpdate(float dt);
    void Update(float dt);
    void LateUpdate(float dt);
    void Render();
    void UIRender();
    void Detach();

   protected:
    const std::string debug_name;
    const UUID uuid;

    App* ParentApp() const { return parent_app; }

   private:
    App* parent_app = nullptr;

    virtual void OnAttach() {}
    virtual void OnDetach() {}
    virtual void OnEarlyUpdate(float dt) {}
    virtual void OnUpdate(float dt) {}
    virtual void OnLateUpdate(float dt) {}
    virtual void OnRender() {}
    virtual void OnUIRender() {}
  };

  template <typename T>
  concept layer_type = std::is_base_of_v<Layer, T>;

}  // namespace other

#endif  // !OTHER_ENGINE_LAYER_HPP

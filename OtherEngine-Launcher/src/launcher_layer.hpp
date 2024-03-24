/**
 * \file launcher_layer.hpp
 **/
#ifndef OTHER_ENGINE_LAUNCHER_LAYER_HPP
#define OTHER_ENGINE_LAUNCHER_LAYER_HPP

#include "core/layer.hpp"
#include "rendering/ui/ui_window.hpp"

namespace other {

  struct SelectionContext {
    Opt<std::string> selected_path;
  };

  class LauncherLayer : public Layer {
    public:
      LauncherLayer(App* parent_app)
        : Layer(parent_app , "LauncherLayer") {}
      virtual ~LauncherLayer() override {}

      virtual void OnAttach() override;
      virtual void OnUpdate(float dt) override;
      virtual void OnRender() override;
      virtual void OnUIRender() override;

    private:
      Ref<UIWindow> window = nullptr;
      SelectionContext selection_context;

      std::unordered_map<uint64_t , Ref<UIWindow>> windows;
  };

} // namespace other

#endif // !OTHER_ENGINE_LAUNCHER_LAYER_HPP

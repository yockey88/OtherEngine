/**
 * \file editor/panels/scene_renderer_settings.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_RENDERER_SETTINGS_HPP
#define OTHER_ENGINE_SCENE_RENDERER_SETTINGS_HPP

#include "editor/editor_panel.hpp"

namespace other {

  class SceneRendererSettings : public EditorPanel {
   public:
    SceneRendererSettings() {}
    virtual ~SceneRendererSettings() override {}

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual bool OnGuiRender(bool& is_open) override;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_SCENE_RENDERER_SETTINGS_HPP
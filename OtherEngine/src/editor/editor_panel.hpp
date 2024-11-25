/**
 * \file editor/editor_panel.hpp
 **/
#ifndef OTHER_ENGINE_EDITOR_PANEL_HPP
#define OTHER_ENGINE_EDITOR_PANEL_HPP

#include "core/ref.hpp"
#include "core/ref_counted.hpp"

#include "event/event.hpp"
#include "project/project.hpp"

#include "scene/scene.hpp"

namespace other {

  class Editor;

  class EditorPanel : public RefCounted {
   public:
    EditorPanel() {}
    virtual ~EditorPanel() {}

    virtual void OnAttach() {}
    virtual void OnDetach() {}

    virtual bool OnGuiRender(bool& is_open) { return false; }

    virtual void OnEarlyUpdate(float dt) {}
    virtual void OnUpdate(float dt) {}
    virtual void OnLateUpdate(float dt) {}

    virtual void OnRender() {}
    virtual void OnRenderUI() {}

    virtual void OnProjectChange(const Ref<Project>& project) {}
    virtual void SetSceneContext(const Ref<Scene>& scene) {}
    virtual void OnScriptReload() {}
  };

}  // namespace other

#endif  // !OTHER_ENGINE_EDITOR_PANEL_HPP

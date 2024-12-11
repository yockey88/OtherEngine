/**
 * \file editor/panels/framebuffer_editor.hpp
 **/
#ifndef OTHER_ENGINE_FRAMEBUFFER_EDITOR_HPP
#define OTHER_ENGINE_FRAMEBUFFER_EDITOR_HPP

#include "editor/editor_panel.hpp"

namespace other {

  class FramebufferEditor : public EditorPanel {
   public:
    FramebufferEditor() {}
    virtual ~FramebufferEditor() override {}

    virtual void OnAttach() override;

    virtual bool OnGuiRender(bool& is_open) override;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_FRAMEBUFFER_EDITOR_HPP
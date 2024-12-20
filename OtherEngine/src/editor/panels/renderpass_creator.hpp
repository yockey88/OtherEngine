/**
 * \file editor/panels/renderpass_creator.hpp
 **/
#ifndef OTHER_ENGINE_RENDERPASS_CREATOR_HPP
#define OTHER_ENGINE_RENDERPASS_CREATOR_HPP

#include "rendering/render_pass.hpp"

#include "editor/editor_panel.hpp"

namespace other {

  class RenderpassCreator : public EditorPanel {
   public:
    RenderpassCreator() = default;
    virtual ~RenderpassCreator() override {}

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual bool OnGuiRender(bool& is_open) override;

   private:
    RenderPassSpec spec;

    std::array<char, 64> pass_name;

    std::map<UUID, Ref<FileHandle>> other_shader_files;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_RENDERPASS_CREATOR_HPP

/**
 * \file editor/panels/pipeline_creator.hpp
 **/
#ifndef OTHER_ENGINE_PIPELINE_CREATOR_HPP
#define OTHER_ENGINE_PIPELINE_CREATOR_HPP

#include "core/ref.hpp"

#include "scene/scene.hpp"

#include "editor/editor_panel.hpp"

namespace other {

  class PipelineCreator : public EditorPanel {
   public:
    PipelineCreator() {}
    virtual ~PipelineCreator() override {}

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual bool OnGuiRender(bool& is_open) override;

   private:
    PipelineSpec spec{};

    std::array<char, 64> pipeline_name{};

    const char* draw_modes[7] = { "Points", "Lines", "Line Strip", "Line Loop", "Triangles", "Triangle Strip", "Triangle Fan" };
    uint32_t draw_mode_count = 7;
    /// this works for this one because the GL_ draw mode enums start at 0 and are contiguous
    ///  other GL_ draw mode enums don't start at 0/may not be contiguous
    uint32_t selected_draw_mode = DrawMode::TRIANGLES;

    enum DepthFunctionIndices {
      NEVER = 0,
      LESS,
      EQUAL,
      LESS_EQUAL,
      GREATER,
      NOT_EQUAL,
      GREATER_EQUAL,
      ALWAYS
    };

    const char* depth_funcs[8] = { "Never", "Less", "Equal", "Less Equal", "Greater", "Not Equal", "Greater Equal", "Always" };
    uint32_t depth_func_count = 8;
    uint32_t selected_depth_func = DepthFunctionIndices::LESS;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_PIPELINE_CREATOR_HPP

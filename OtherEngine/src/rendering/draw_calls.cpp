/**
 * \rendering/draw_calls.cpp
 **/
#include "rendering/draw_calls.hpp"

namespace other {

  RenderStaticSubmission::operator MeshKey() const {
    return {
      .source_handle = model->GetModelSource()->handle,
      .render_state = render_state,
      .draw_mode = draw_mode,
      .line_thickness = line_thickness,
    };
  }

  RenderSubmission::operator MeshKey() const {
    return {
      .source_handle = model->GetModelSource()->handle,
      .render_state = render_state,
      .draw_mode = draw_mode,
      .line_thickness = line_thickness,
    };
  }

}  // namespace other
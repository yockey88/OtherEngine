/**
 * \rendering/draw_calls.cpp
 **/
#include "rendering/draw_calls.hpp"

namespace other {

  RenderStaticSubmission::operator MeshKey() const {
    return {
      .source_handle = model->GetModelSource()->handle,
      .render_state = render_state,
    };
  }

  RenderSubmission::operator MeshKey() const {
    return {
      .source_handle = model->GetModelSource()->handle,
      .render_state = render_state,
    };
  }

}  // namespace other
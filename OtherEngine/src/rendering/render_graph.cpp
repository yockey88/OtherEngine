/**
 * \file rendering/render_graph.cpp
 **/
#include "rendering/render_graph.hpp"

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

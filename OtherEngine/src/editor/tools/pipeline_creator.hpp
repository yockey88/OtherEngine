/**
 * \file editor/tools/pipeline_creator.hpp
 **/
#ifndef OTHER_ENGINE_PIPELINE_CREATOR_HPP
#define OTHER_ENGINE_PIPELINE_CREATOR_HPP

#include "rendering/ui/ui_window.hpp"

namespace other {

  class PipelineCreatorWindow : public UIWindow {
   public:
    PipelineCreatorWindow()
        : UIWindow("Rendering Pipeline Creator") {}
    ~PipelineCreatorWindow() override {}
  };

}  // namespace other

#endif  // !OTHER_ENGINE_PIPELINE_CREATOR_HPP

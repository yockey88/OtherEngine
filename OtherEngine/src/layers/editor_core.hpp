/**
 * \file layers/editor_core.hpp
 */
#ifndef OTHER_ENGINE_EDITOR_CORE_HPP
#define OTHER_ENGINE_EDITOR_CORE_HPP

#include "core/defines.hpp"
#include "core/layer.hpp"

namespace other {

  constexpr UUID kEditorCoreUUID = FNV("EditorCore");

  class EditorCore : public Layer {
    public:
      EditorCore(App* parent_app)
        : Layer(parent_app , "EditorCore") {}
      ~EditorCore() = default;
    private:
  };

} // namespace other

#endif // !OTHER_ENGINE_EDITOR_CORE_HPP


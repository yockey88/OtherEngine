/**
 * \file launcher_layer.hpp
 **/
#ifndef OTHER_ENGINE_LAUNCHER_LAYER_HPP
#define OTHER_ENGINE_LAUNCHER_LAYER_HPP

#include "core/layer.hpp"
#include "core/uuid.hpp"
#include "core/platform.hpp"
#include "scripting/script_object.hpp"

namespace other {

  enum SelectionContextType {
    CREATE_PROJECT,
    OPEN_PROJECT,
    SAVE_PROJECT,

    INVALID_CONTEXT
  };

  struct SelectionContext {
    Opt<std::string> selected_path;
    SelectionContextType context_type = INVALID_CONTEXT;
  };

  class LauncherLayer : public Layer {
    public:
      LauncherLayer(App* parent_app)
        : Layer(parent_app , "LauncherLayer") {}
      virtual ~LauncherLayer() override {}

      virtual void OnAttach() override;
      virtual void OnUpdate(float dt) override;
      virtual void OnRender() override;
      virtual void OnUIRender() override;
      virtual void OnEvent(Event* event) override;
      virtual void OnDetach() override;

    private:
      enum class RenderState {
        MAIN = 0 ,
        OPENING_PROJECT ,
        CREATING_PROJECT ,

        NUM_RENDER_STATES ,
        INVALID_RENDER_STATE = NUM_RENDER_STATES
      };

      RenderState render_state = RenderState::INVALID_RENDER_STATE;

      UUID editor_id = 0;
      UUID create_proj_id = 0;
      bool rendering_open_project = true;

      SelectionContext selection_context;

      ScriptObject* script = nullptr;
      ScriptObject* lua_script = nullptr;

      void CreateProject();
      void OpenProject();

      void RenderMain();
      void RenderOpeningProject();

      void Launch(const std::filesystem::path& path , LaunchType type);
  };

} // namespace other

#endif // !OTHER_ENGINE_LAUNCHER_LAYER_HPP

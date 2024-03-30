/**
 * \file create_project.hpp
 **/
#ifndef OTHER_ENGINE_CREATE_PROJECT_HPP
#define OTHER_ENGINE_CREATE_PROJECT_HPP

#include <string>
#include <array>
#include <filesystem>

#include "core/defines.hpp"
#include "core/layer.hpp"
#include "core/uuid.hpp"
#include "rendering/ui/text_editor.hpp"

namespace other {

  constexpr std::string_view kCreateProjectLayerName = "CreateProjectLayer";
  constexpr UUID kCreateProjectLayerUUID = FNV("CreateProjectLayer");

  class CreateProjectLayer : public Layer {
    public:
      CreateProjectLayer(App* parent_app , const std::string& path)
        : Layer(parent_app , std::string{ kCreateProjectLayerName }) , project_path(path) {}

      virtual ~CreateProjectLayer() override {}

      virtual void OnAttach() override;
      virtual void OnUpdate(float dt) override;
      virtual void OnEvent(Event* event) override;
      virtual void OnUIRender() override;

      std::string ProjectName() const { return project_name.value_or(""); }
      std::string ProjectPath() const { return project_path; }

    private:
      enum class RenderState {
        MAIN ,
        CREATING_PROJECT ,

        NUM_STATES ,
        INVALID_STATE = NUM_STATES
      };

      RenderState render_state = RenderState::INVALID_STATE;
      bool rendering_main = true;
      bool rendering_create_project = true;

      std::string project_path;
      Opt<std::string> project_name;
      Opt<std::string> author;
      Opt<std::string> description;

      std::string final_name;
      std::filesystem::path project_dir;
      std::filesystem::path project_file;

      constexpr static size_t kBufferSize = 256;
      std::array<char , kBufferSize> project_name_buffer = {0};
      std::array<char , kBufferSize> author_buffer = {0};
      std::array<char , kBufferSize> description_buffer = {0};

      std::vector<std::filesystem::path> project_dir_contents;

      Ref<TextEditor> text_editor = nullptr;

      UUID text_editor_id = 0;

      void RenderDirContents();

      void RenderMain();
      void CreatingProject();

      std::string CaptureBuffer(const std::array<char , kBufferSize>& buffer);
  };

} // namespace other

#endif // !OTHER_ENGINE_CREATE_PROJECT_HPP

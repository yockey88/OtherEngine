/**
 * \file editor/panels/file_editor.hpp
 **/
#ifndef OTHER_ENGINE_FILE_EDITOR_HPP
#define OTHER_ENGINE_FILE_EDITOR_HPP

#include "core/directory.hpp"
#include "core/file_handle.hpp"

#include "rendering/ui/text_editor.hpp"

#include "editor/editor_panel.hpp"

namespace other {

  class FileEditor : public EditorPanel {
   public:
    FileEditor() {}
    virtual ~FileEditor() {}

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate(float dt) override;

    virtual bool OnGuiRender(bool& is_open) override;

   private:
    Ref<Directory> project_root = nullptr;
    Ref<Directory> game_root = nullptr;

    Opt<std::array<char, 256>> new_file_name = std::nullopt;

    Ref<TextEditor> editor = nullptr;

    void RenderDirectory(Ref<Directory>& dir);
    void RenderFile(Ref<FileHandle>& file);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_FILE_EDITOR_HPP

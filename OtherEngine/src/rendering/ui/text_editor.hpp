/**
 * \file rendering/ui/text_editor.hpp
 **/
#ifndef OTHER_ENGINE_TEXT_EDITOR_HPP
#define OTHER_ENGINE_TEXT_EDITOR_HPP

#include <memory>
#include <filesystem>

#include <zep/editor.h>
#include <zep/imgui/console_imgui.h>

#include "rendering/ui/ui_window.hpp"

namespace Zep {

  using MessagePtr = std::shared_ptr<ZepMessage>;

} // namespace Zep

namespace other {

  class ZepWrapper final : public Zep::IZepComponent  {
    std::function<void(Zep::MessagePtr)> message_callback;

    public:
      Scope<Zep::ZepEditor_ImGui> editor;

      ZepWrapper(
        const std::string& config_path , const Zep::NVec2f& pixel_scale , 
        std::function<void(Zep::MessagePtr)> message_callback
      ) : message_callback(message_callback) ,  editor(NewScope<Zep::ZepEditor_ImGui>(config_path , pixel_scale)) {
        editor->RegisterCallback(this);
      }

      virtual Zep::ZepEditor& GetEditor() const override { 
        return (Zep::ZepEditor&)editor; 
      }

      virtual void Notify(std::shared_ptr<Zep::ZepMessage> message) override {
        message_callback(message);
      }

      virtual void HandleInput() {
        editor->HandleInput();
      }

      virtual void Destroy() {
        editor->UnRegisterCallback(this);
        editor = nullptr;
      }
  };

  class TextEditor : public UIWindow {
    Zep::ZepBuffer* buffer = nullptr;
    Scope<ZepWrapper> zep;
    Zep::Msg MsgIdFromString(const std::string& msg);

    std::function<void(Zep::MessagePtr)> callback 
      = [this](Zep::MessagePtr message) {
          zep->editor->Notify(message);
          if (message->str == "RequestQuit" || message->str == ":wq") {
            Close();
          }
        };

    std::string root;
    std::string file;
    std::string file_path;
    Zep::NVec2f pixel_scale;

    bool file_exists = false;

    void CreateEditor();

    public:
      TextEditor(
        const std::string& title , const std::string& root , const std::string& file ,
        const Zep::NVec2f& pixel_scale = Zep::NVec2f(1.0f , 1.0f)
      ) : UIWindow(title) , root(root) , file(file) , file_path(root + "/" + file) , pixel_scale(pixel_scale) {}
      virtual ~TextEditor() override {}

      virtual void OnAttach() override;
      virtual void OnDetach() override;
      virtual void OnUpdate(float dt) override;
      virtual void Render() override;

      void LoadFile(const std::string& file);
  }; 

} // namespace other

#endif // !OTHER_ENGINE_TEXT_EDITOR_HPP

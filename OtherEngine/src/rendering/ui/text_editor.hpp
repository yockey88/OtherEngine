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

  using MessageFunc = std::function<void(const std::vector<std::string>&)>;

} // namespace Zep

namespace other {

  class ZepCmd final : public Zep::ZepExCommand {
    public:
      ZepCmd(Zep::ZepEditor& editor , const std::string& name , Zep::MessageFunc callback) 
        : Zep::ZepExCommand(editor) , name(name) , callback(callback) {}

      virtual void Run(const std::vector<std::string>& commands) override {
        callback(commands);
      }

      virtual const char* ExCommandName() const override {
        return name.c_str();
      };

    private:
      std::string name;
      Zep::MessageFunc callback;

  };

  class ZepWrapper final : public Zep::IZepComponent  {
    public:
      Scope<Zep::ZepEditor_ImGui> editor;

      ZepWrapper(
        const std::string& config_path , const Zep::NVec2f& pixel_scale , 
        std::function<void(Zep::MessagePtr)> message_callback
      ) : editor(NewScope<Zep::ZepEditor_ImGui>(config_path , pixel_scale)) , 
          message_callback(message_callback) {
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

    private:
      std::function<void(Zep::MessagePtr)> message_callback;
  };

  class TextEditor : public UIWindow {
    public:
      TextEditor(
        const std::string& title , const std::string& root , const std::string& file ,
        const Zep::NVec2f& pixel_scale = Zep::NVec2f(1.0f , 1.0f)
      ) : UIWindow(title) , root(root) , file(file) , file_path(file) , pixel_scale(pixel_scale) {}
      virtual ~TextEditor() override {}

      virtual void OnAttach() override;
      virtual void OnDetach() override;
      virtual void OnUpdate(float dt) override;
      virtual void OnEvent(Event* event) override;
      virtual void Render() override;

      void LoadFile(const std::string& file);

      std::string GetFile() const { return file_path; }

    private:
      Zep::ZepBuffer* buffer = nullptr;
      Scope<ZepWrapper> zep;
      Zep::Msg MsgIdFromString(const std::string& msg);

      std::function<void(Zep::MessagePtr)> callback = std::bind_front(&TextEditor::HandleMessage , this); 

      std::string root;
      std::string file;
      std::string file_path;
      Zep::NVec2f pixel_scale;

      bool has_focus = false;
      bool file_exists = false;

      void CreateEditor();
      void HandleMessage(Zep::MessagePtr message);
  }; 

} // namespace other

#endif // !OTHER_ENGINE_TEXT_EDITOR_HPP

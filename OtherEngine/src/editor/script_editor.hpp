/**
 * \file editor/script_editor.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_EDITOR_HPP
#define OTHER_ENGINE_SCRIPT_EDITOR_HPP

#include <functional>
#include <string>

#include "rendering/ui/text_editor.hpp"

#include "scripting/script_module.hpp"

namespace other {

  class ScriptZepCmd final : public Zep::ZepExCommand {
    public:
      ScriptZepCmd(Zep::ZepEditor& editor , const std::string& name , Zep::MessageFunc callback) 
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

  class ScriptZepWrapper final : public Zep::IZepComponent  {
    public:
      ScriptZepWrapper(const std::string& config_path , const Zep::NVec2f& pixel_scale , 
                       std::function<void(Zep::MessagePtr)> message_callback) 
          : editor(NewScope<Zep::ZepEditor_ImGui>(config_path , pixel_scale)) , message_callback(message_callback) {
        editor->RegisterCallback(this);
      }

      virtual Zep::ZepEditor& GetEditor() const override { 
        return *editor; 
      }

      virtual void Notify(std::shared_ptr<Zep::ZepMessage> message) override {
        editor->Notify(message);
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
      Scope<Zep::ZepEditor_ImGui> editor;
      std::function<void(Zep::MessagePtr)> message_callback;
  };

  class ScriptEditor : public UIWindow {
    public:
      ScriptEditor(Opt<std::string> window_name = std::nullopt);

      void AddEditor(const ScriptObjectTag& obj_tag , const std::string& script_path);

      virtual void OnUpdate(float dt) override;
      virtual void Render() override;
      void PostRender();

    private:
      std::string script_path;

      Zep::NVec2f pixel_scale = Zep::NVec2f(1.f , 1.f);

      struct TextBuffer {
        UUID id;
        bool tab_open = false;
        ScriptEditor* editor = nullptr;
        Zep::ZepBuffer* text_buffer = nullptr;
        Scope<ScriptZepWrapper> zep = nullptr;
        std::function<void(Zep::MessagePtr)> callback = nullptr;

        void HandleMessage(Zep::MessagePtr);
      };

      std::vector<UUID> ids_to_clear;
      std::map<UUID , TextBuffer> active_editors;
  };


} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_EDITOR_HPP

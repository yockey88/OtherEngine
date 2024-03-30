/**
 * \file editor/editor.cpp
*/
#include "editor/editor.hpp"

#include "core/engine.hpp"
#include "core/errors.hpp"
#include "core/logger.hpp"
#include "event/event_queue.hpp"
#include "event/core_events.hpp"
#include "event/key_events.hpp"
#include "project/project.hpp"
#include "parsing/ini_parser.hpp"
#include "rendering/renderer.hpp"

namespace other {
  
  Editor::Editor(Engine* engine , Scope<App>& app)
      : App(engine) , cmdline(engine->cmd_line) , engine_config(engine->config) , 
        app(std::move(app)) {
    project = Project::Create(engine->cmd_line , engine->config);     
  } 

  void Editor::OnAttach() {
    if (app == nullptr) {
      OE_ERROR("Attempting to attach a null application to the editor");
      EventQueue::PushEvent<ShutdownEvent>(ExitCode::FAILURE);
      return;
    }

    try {
      std::filesystem::path p = std::filesystem::absolute("OtherEngine/editor.other");
      IniFileParser parser(p.string());
      editor_config = parser.Parse();

    } catch (const IniException& e) {
      OE_ERROR("Failed to parse editor configuration file : {}", e.what());
      EventQueue::PushEvent<ShutdownEvent>(ExitCode::FAILURE);
      return;
    }

    Renderer::ClearColor(editor_config.Get("WINDOW" , "CLEAR-COLOR"));
  }

  void Editor::OnEvent(Event* event) {
  }

  void Editor::Update(float dt) {
  }

  void Editor::Render() {
  }

  void Editor::RenderUI() {
  }

  void Editor::OnDetach() {
  }

} // namespace other

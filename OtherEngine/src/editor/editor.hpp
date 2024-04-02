/**
 * \file editor/editor.hpp
 */
#ifndef OTHER_ENGINE_EDITOR_HPP
#define OTHER_ENGINE_EDITOR_HPP

#include "core/layer.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "project/project.hpp"
#include "application/app.hpp"

namespace other {

  class Editor : public App {
    public:
      Editor(Engine* engine , Scope<App>& app);
      ~Editor() = default;

    private:
      const CmdLine& cmdline;
      const ConfigTable& engine_config;
      ConfigTable editor_config;

      Scope<App> app;
      Ref<Project> project;

      virtual void OnAttach() override;
      virtual void OnEvent(Event* event) override;
      virtual void Update(float dt) override;
      virtual void Render() override;
      virtual void RenderUI() override;
      virtual void OnDetach() override;
  };

} // namespace other

#endif // !OTHER_ENGINE_EDITOR_HPP
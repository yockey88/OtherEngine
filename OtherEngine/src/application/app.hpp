/**
 * \file application\app.hpp
 **/
#ifndef OTHER_ENGINE_APP_HPP
#define OTHER_ENGINE_APP_HPP

#include "core/config.hpp"
#include "core/layer.hpp"

#include "asset/asset_handler.hpp"
#include "parsing/cmd_line_parser.hpp"

#include "scene/scene_manager.hpp"

#include "rendering/ui/ui_window_map.hpp"

namespace other {

  class Engine;

  class App {
   public:
    App(const CmdLine& cmdline, const ConfigTable& config);
    virtual ~App();

    virtual Ref<AssetHandler> CreateAssetHandler(EngineMode mode);
    virtual Ref<SceneRenderer> CreateSceneRenderer();

    void Load();
    void Unload();

    void Attach();
    void DoEarlyUpdate(float dt);
    void DoUpdate(float dt);
    void DoLateUpdate(float dt);
    void OnRender();
    void OnRenderUI();
    void Detach();

    struct RenderSpec {
      std::vector<FramebufferSpec> framebuffer_specs;
      std::vector<RenderPassSpec> render_passes;
      std::vector<PipelineSpec> pipelines;
      std::map<UUID, std::vector<UUID>> pipeline_to_pass_map;
    } render_specs;

   protected:
    virtual void OnLoad() {}
    virtual void OnAttach() {}

    virtual void EarlyUpdate(float dt) {}
    virtual void Update(float dt) {}
    virtual void LateUpdate(float dt) {}
    virtual void Render() {}
    virtual void RenderUI() {}

    virtual void OnDetach() {}
    virtual void OnUnload() {}

    virtual void OnSceneLoad(const SceneMetadata* path) {}
    virtual void OnSceneUnload() {}

    /// will only ever be called if editor is active because otherwise the scripts
    ///   wont be reloaded
    virtual void OnScriptReload() {}

    const CmdLine& cmdline;
    const ConfigTable& config;

    UIWindowMap ui_windows;

    friend class Engine;
    friend class Editor;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_APP_HPP

/**
 * \file application\app.hpp
 **/
#ifndef OTHER_ENGINE_APP_HPP
#define OTHER_ENGINE_APP_HPP

#include "core/config.hpp"
#include "core/defines.hpp"
#include "core/layer.hpp"
#include "core/layer_stack.hpp"

#include "asset/asset_handler.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "project/project.hpp"

#include "scene/scene_manager.hpp"

#include "rendering/ui/ui_window.hpp"
#include "rendering/ui/ui_window_map.hpp"

namespace other {

  class Engine;

  class App {
   public:
    App(const CmdLine& cmdline, const ConfigTable& config);
    virtual ~App();

    virtual Ref<AssetHandler> CreateAssetHandler();
    virtual Ref<SceneRenderer> CreateSceneRenderer();

    void Load();
    void Run();
    void Unload();

    void Attach();
    /// this is seperate because this triggers events which need to be polled while update
    ///  responds to process events
    void DoEarlyUpdate(float dt);
    void DoUpdate(float dt);
    void DoLateUpdate(float dt);
    void OnRender();
    void OnRenderUI();
    void Detach();

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

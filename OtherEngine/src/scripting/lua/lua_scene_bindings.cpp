/**
 * \file scripting/lua/lua_scene_bindings.cpp
 **/
#include "scripting/lua/lua_scene_bindings.hpp"

#include "application/app_state.hpp"

namespace other {
namespace lua_script_bindings {
namespace {

  static bool NativeHasActiveScene() {
    return AppState::Scenes()->HasActiveScene();
  }

  static bool NativeSceneRunning() {
    return AppState::Scenes()->IsPlaying();
  }

  static void NativeStartScene() {
    AppState::Scenes()->StartScene();
  }
  
  static void NativeStopScene() {
    AppState::Scenes()->StopScene();
  }

} // anonymous namespace

  void BindScene(sol::state& lua_state) {
    BindSceneManagerFunctions(lua_state);
  }

  void BindSceneManagerFunctions(sol::state& lua_state) {
    sol::table scene_manager = lua_state.create_named_table("SceneManager");
    scene_manager.set_function("HasActiveScene" , NativeHasActiveScene);
    scene_manager.set_function("StartScene" , NativeStartScene);
    scene_manager.set_function("StopScene" , NativeStopScene);
    scene_manager.set_function("SceneRunning" , NativeSceneRunning);
  }

} // namespace lua_script_bindings
} // namespace other

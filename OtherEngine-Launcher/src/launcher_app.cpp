/**
 * \file launcher_app.cpp
 **/
#include "launcher_app.hpp"

#include "application/app_state.hpp"

#include "launcher_layer.hpp"
#include "other_engine.hpp"

namespace other {

  void LauncherApp::OnAttach() {
    Ref<Layer> layer = NewRef<LauncherLayer>(this);
    AppState::PushLayer(layer);
  }

  // void LauncherApp::OnEvent(Event* event) {
  //   EventHandler handler(event);
  //   handler.Handle<KeyPressed>([](KeyPressed& key) -> bool {
  //     if (key.Key() == Keyboard::Key::OE_ESCAPE) {
  //       EventQueue::PushEvent<ShutdownEvent>(ExitCode::SUCCESS);
  //       return true;
  //     }
  //     return false;
  //   });
  // }

}  // namespace other

OE_APPLICATION(LauncherApp);

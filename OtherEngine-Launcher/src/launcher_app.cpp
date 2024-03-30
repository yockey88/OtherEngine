/**
 * \file launcher_app.cpp
 **/
#include "launcher_app.hpp"

#include "other_engine.hpp"
#include "event/core_events.hpp"
#include "event/key_events.hpp"
#include "input/keyboard.hpp"

#include "launcher_layer.hpp"

namespace other {

  void LauncherApp::OnAttach() {
    {
      Ref<Layer> layer = NewRef<LauncherLayer>(this);
      PushLayer(layer);
    }
  }

  void LauncherApp::OnEvent(Event* event) {
    if (event->Type() == EventType::KEY_PRESSED) {
      KeyPressed* key = Cast<KeyPressed>(event);
      if (key != nullptr) {
        if (key->Key() == Keyboard::Key::OE_ESCAPE) {
          EventQueue::PushEvent<ShutdownEvent>(ExitCode::SUCCESS);
        }
      }
    }
  }

} // namespace other

OE_APPLICATION(LauncherApp);

/**
 * \file launcher_app.cpp
 **/
#include "launcher_app.hpp"

#include "other_engine.hpp"

#include "event/core_events.hpp"
#include "event/key_events.hpp"
#include "event/event_handler.hpp"
#include "event/event_queue.hpp"

#include "input/keyboard.hpp"

#include "launcher_layer.hpp"

namespace other {

  void LauncherApp::OnAttach() {
    Ref<Layer> layer = NewRef<LauncherLayer>(this);
    PushLayer(layer);
  }

  void LauncherApp::OnEvent(Event* event) {
    EventHandler handler(event);
    handler.Handle<KeyPressed>([](KeyPressed& key) -> bool {
      if (key.Key() == Keyboard::Key::OE_ESCAPE) {
        EventQueue::PushEvent<ShutdownEvent>(ExitCode::SUCCESS);
        return true;
      }
      return false;
    });
  }

} // namespace other

OE_APPLICATION(LauncherApp);

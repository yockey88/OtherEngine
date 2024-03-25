/**
 * \file testbed.cpp
 */
#include "testbed.hpp"

#include "other_engine.hpp"
#include "event/core_events.hpp"
#include "event/key_events.hpp"

namespace other {

  void Testbed::OnAttach() {
  }

  void Testbed::OnEvent(Event* event) {
    if (event->Type() == EventType::KEY_PRESSED) {
      KeyPressed* key = Cast<KeyPressed>(event);
      if (key != nullptr) {
        if (key->Key() == Keyboard::Key::KEY_ESCAPE) {
          EventQueue::PushEvent<ShutdownEvent>(ExitCode::RELOAD_PROJECT);
        }
      }
    }
  }

  void Testbed::Update(float dt) {
  }

} // namespace other

OE_APPLICATION(Testbed);

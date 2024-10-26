/**
 * \file control_layer.cpp
 **/
#include "control_layer.hpp"

#include "event/app_events.hpp"
#include "event/event_queue.hpp"
#include "event/key_events.hpp"
#include "event/window_events.hpp"
#include "input/mouse.hpp"

void ControlLayer::OnAttach() {
  EventQueue::RegisterEventDispatcher<KeyPressed>(
    "Key-Bindings",
    {
      std::bind_front(&ControlLayer::HandleKeyPress, this),
    }
  );

  EventQueue::RegisterEventDispatcher<WindowClosed>(
    "Close-Window",
    {
      [&](WindowClosed& event) -> bool {
        running = false;
        return true;
      },
    }
  );

  Mouse::FreeCursor();
}

bool ControlLayer::HandleKeyPress(KeyPressed& event) {
  HandleKeyEvent(event, Keyboard::Key::OE_C, [&](KeyPressed& event) {
    camera_lock = !camera_lock;
    if (camera_lock) {
      Mouse::FreeCursor();
    } else {
      Mouse::LockCursor();
    }
  });

  HandleKeyEvent(event, Keyboard::Key::OE_ESCAPE, [&](KeyPressed& event) {
    // EventQueue::PushEvent<ShutdownEvent>({ ExitCode::SUCCESS });
    running = false;
  });

  HandleKeyEvent(event, Keyboard::Key::OE_R, [&](KeyPressed& event) {
    EventQueue::PushEvent<ScriptReloadEvent>();
  });

  return !running;
}
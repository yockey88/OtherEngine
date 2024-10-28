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
  EventQueue::RegisterEventDispatcher<WindowClosed>(
    "Close-Window",
    { std::bind_front(&ControlLayer::HandleWindowClosed, this) }
  );

  EventQueue::RegisterEventDispatcher<KeyPressed>(
    "Control-Layer-Key-Bindings",
    { std::bind_front(&ControlLayer::HandleKeyPress, this) }
  );

  Mouse::FreeCursor();
}

bool ControlLayer::HandleWindowClosed(WindowClosed& event) {
  EventQueue::PushEvent<ShutdownEvent>({ ExitCode::SUCCESS });
  return true;
}

bool ControlLayer::HandleKeyPress(KeyPressed& event) {
  HandleKeyEvent(event, Keyboard::Key::OE_ESCAPE, [&](KeyPressed& event) {
    EventQueue::PushEvent<ShutdownEvent>({ ExitCode::SUCCESS });
  });

  HandleKeyEvent(event, Keyboard::Key::OE_R, [&](KeyPressed& event) {
    EventQueue::PushEvent<ScriptReloadEvent>();
  });

  return false;
}
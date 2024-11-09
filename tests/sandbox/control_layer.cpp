/**
 * \file control_layer.cpp
 **/
#include "control_layer.hpp"

#include "event/app_events.hpp"
#include "event/event_queue.hpp"
#include "event/key_events.hpp"
#include "input/mouse.hpp"

void ControlLayer::OnAttach() {
  EventQueue::RegisterEventDispatcher<KeyPressed>(
    "ControlLayer--KeyPressed",
    { std::bind_front(&ControlLayer::HandleKeyPress, this) }
  );

  Mouse::FreeCursor();
}

bool ControlLayer::HandleKeyPress(KeyPressed& event) {
  HandleKeyEvent(event, Keyboard::Key::OE_R, [&]() {
    EventQueue::PushEvent<ScriptReloadEvent>();
  });

  return false;
}
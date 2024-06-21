/**
 * \file layers/editor_core.cpp
 */
#include "layers/editor_core.hpp"

#include "event/scene_events.hpp"
#include "event/event_handler.hpp"

namespace other {

  void EditorCore::OnEvent(Event* event) {
    EventHandler handler(event);
    handler.Handle<SceneLoad>([this](SceneLoad& scene_load_event) -> bool {
      return true;
    });
  }

} // namespace other

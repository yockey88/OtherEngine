/**
 * \file layers/editor_core.cpp
 */
#include "layers/editor_core.hpp"

#include "core/logger.hpp"
#include "application/app.hpp"
#include "event/scene_events.hpp"

namespace other {

  void EditorCore::OnEvent(Event* event) {
    if (event->Type() == EventType::SCENE_LOAD) {
      SceneLoad* scene_load = Cast<SceneLoad>(event);
      OE_ASSERT(scene_load != nullptr , "Event cast to 'SceneLoad' failed");

      event->handled = true;
    } 
  }

} // namespace other

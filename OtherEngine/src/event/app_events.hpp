/**
 * \file event/app_events.hpp
 */
#ifndef OTHER_ENGINE_APP_EVENTS_HPP
#define OTHER_ENGINE_APP_EVENTS_HPP

#include <sstream>

#include "event/core_events.hpp"
#include "event/event.hpp"
#include "project/project.hpp"

namespace other {

  struct AppLayerEvent {
    LAYER_EVENT();
    EVENT_TYPE(APP_LAYER);
  };
  static_assert(sizeof(AppLayerEvent) == 16, "AppLayerEvent size is not correct");

  struct ScriptReload {
    EVENT_CATEGORY(APPLICATION_EVENT);
    EVENT_TYPE(SCRIPT_RELOAD);
  };
  static_assert(sizeof(ScriptReload) == 1, "ScriptReload size is not correct");

  struct ProjectDirectoryUpdateEvent {
    ProjectDirectoryType dir_type;
    EVENT_CATEGORY(APPLICATION_EVENT);
    EVENT_TYPE(PROJECT_DIR_UPDATE);
  };
  static_assert(sizeof(ProjectDirectoryUpdateEvent) == 4, "ProjectDirectoryUpdateEvent size is not correct");

  static_assert(Event<AppLayerEvent>, "AppLayerEvent does not meet the Event concept");
  static_assert(Event<ScriptReload>, "ScriptReload does not meet the Event concept");
  static_assert(Event<ProjectDirectoryUpdateEvent>, "ProjectDirectoryUpdateEvent does not meet the Event concept");

}  // namespace other

#endif  // !OTHER_ENGINE_APP_EVENTS_HPP

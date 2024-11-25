/**
 * \file event/scene_events.hpp
 */
#ifndef OTHER_ENGINE_SCENE_EVENTS_HPP
#define OTHER_ENGINE_SCENE_EVENTS_HPP

#include "event/event.hpp"

namespace other {

#define SCENE_EVENT() EVENT_CATEGORY(APPLICATION_EVENT | SCENE_EVENT);

  /// \todo fix this, string is corrupted
  struct SceneLoad {
    SCENE_EVENT();
    EVENT_TYPE(SCENE_LOAD);

    uint64_t scene_id;

    std::string ToString() const {
      return "SceneLoadEvent";
    }
  };

  struct SceneActivate {
    SCENE_EVENT();
    EVENT_TYPE(SCENE_ACTIVATE);

    uint64_t scene_id;

    std::string ToString() const {
      return "SceneActivateEvent";
    }
  };

  struct SceneStart {
    SCENE_EVENT();
    EVENT_TYPE(SCENE_START);

    uint64_t scene_id;

    std::string ToString() const {
      return "SceneStartEvent";
    }
  };

  struct SceneStop {
    SCENE_EVENT();
    EVENT_TYPE(SCENE_STOP);

    uint64_t scene_id;

    std::string ToString() const {
      return "SceneStopEvent";
    }
  };

  struct SceneUnload {
    SCENE_EVENT();
    EVENT_TYPE(SCENE_UNLOAD);

    uint64_t scene_id;

    std::string ToString() const {
      return "SceneUnloadEvent";
    }
  };

  static_assert(Event<SceneLoad>, "SceneLoad does not meet the Event concept");
  static_assert(Event<SceneActivate>, "SceneActivate does not meet the Event concept");
  static_assert(Event<SceneStart>, "SceneStart does not meet the Event concept");
  static_assert(Event<SceneStop>, "SceneStop does not meet the Event concept");
  static_assert(Event<SceneUnload>, "SceneUnload does not meet the Event concept");

}  // namespace other

#endif  // !OTHER_ENGINE_SCENE_EVENTS_HPP

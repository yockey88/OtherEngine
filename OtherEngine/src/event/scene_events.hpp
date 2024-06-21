/**
 * \file event/scene_events.hpp
*/
#ifndef OTHER_ENGINE_SCENE_EVENTS_HPP
#define OTHER_ENGINE_SCENE_EVENTS_HPP

#include "event/event.hpp"

namespace other {

  class SceneEvent : public Event {
    public:
      SceneEvent() = default;

      EVENT_CATEGORY(APPLICATION_EVENT | SCENE_EVENT);
  };

  /// \todo fix this, string is corrupted
  class SceneLoad : public SceneEvent {
    public:
      SceneLoad(const std::string& p) {} 

      ~SceneLoad() {}

      virtual std::string ToString() const override {
        return "SceneLoadEvent";
      }

      EVENT_TYPE(SCENE_LOAD);

    private:
  };

  class SceneStart : public SceneEvent {
    public:
      SceneStart() = default;

      virtual std::string ToString() const override {
        return "SceneStartEvent";
      }

      EVENT_TYPE(SCENE_START);
  };

  class SceneStop : public SceneEvent {
    public:
      SceneStop() = default;

      virtual std::string ToString() const override {
        return "SceneStopEvent";
      }

      EVENT_TYPE(SCENE_STOP);
  };

  class SceneUnload : public SceneEvent {
    public:
      SceneUnload() = default;

      virtual std::string ToString() const override {
        return "SceneUnloadEvent";
      }

      EVENT_TYPE(SCENE_UNLOAD);
  };

} // namespace other

#endif // !OTHER_ENGINE_SCENE_EVENTS_HPP

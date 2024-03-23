/**
 * \file event/event.hpp
*/
#ifndef OTHER_ENGINE_EVENT_HPP
#define OTHER_ENGINE_EVENT_HPP

#include <string>

#include "core/defines.hpp"

namespace other {

  enum class EventType {
    EMPTY = 0 ,
    // window events
    WINDOW_CLOSE , WINDOW_MINIMIZE , WINDOW_RESIZE , WINDOW_FOCUS , WINDOW_LOST_FOCUS , WINDOW_MOVED ,
    // application events
    APP_TICK , APP_UPDATE , APP_RENDER ,
    // input events
    KEY_PRESSED , KEY_RELEASED , KEY_TYPED , KEY_HELD ,
    MOUSE_BUTTON_PRESSED , MOUSE_BUTTON_RELEASED , MOUSE_BUTTON_HELD , MOUSE_MOVD , MOUSE_SCROLLED ,
    // scene events
    SCENE_LOAD , SCENE_START , SCENE_STOP , SCENE_UNLOAD ,
    // editor events
    EDITOR_SCENE_PLAY , EDITOR_SCENE_PAUSE , EDITOR_SCENE_STOP ,

    SHUTDOWN
  };

  enum EventCategory {
    NONE = 0 ,
    APPLICATION_EVENT  = bit(0) ,
    WINDOW_EVENT       = bit(1) ,
    INPUT_EVENT        = bit(2) ,
    KEYBOARD_EVENT     = bit(3) ,
    MOUSE_EVNT         = bit(4) ,
    MOUSE_BUTTON_EVENT = bit(5) ,
    SCENE_EVENT        = bit(6) ,
    EDITOR_EVENT       = bit(7) ,
    SHUTDOWN_EVENT     = bit(8)
  };

#define EVENT_TYPE(type) static EventType GetStaticType() { return EventType::type; }  \
                         virtual EventType Type() const override { return GetStaticType(); } \
                         virtual std::string Name() const override { return #type; } \
                         virtual uint32_t Size() const override { return sizeof(*this); }

#define EVENT_CATEGORY(category) virtual uint32_t CategoryFlags() const override { return category; }

  class Event {
    public:
      Event() {}
      virtual ~Event() {}

      virtual EventType Type() const = 0;
      virtual std::string Name() const = 0;
      virtual uint32_t CategoryFlags() const = 0;
      virtual uint32_t Size() const = 0;

      virtual std::string ToString() const { return Name(); }
      inline bool InCategory(EventCategory category) const { return CategoryFlags() & category; }

      bool handled = false;
  };

  template <typename T>
  T* Cast(Event* event) {
    if (event->Type() == T::GetStaticType()) {
      return (T*)event;
    }

    return nullptr;
  }

  inline std::ostream& operator<<(std::ostream& os , const Event& e) {
    return os << e.Name();
  }

  template <typename T>
  concept event_t = std::derived_from<T , Event>;

} // namespace other

#endif // !OTHER_ENGINE_EVENT_HPP

/**
 * \file event/event.hpp
 */
#ifndef OTHER_ENGINE_EVENT_HPP
#define OTHER_ENGINE_EVENT_HPP

#include <string>

#include "core/defines.hpp"

namespace other {

  enum EventType : int64_t {
    EMPTY_EVNT = 0,

    // window events
    WINDOW_CLOSE = bit(1),
    WINDOW_MINIMIZE = bit(2),
    WINDOW_RESIZE = bit(3),
    /// \todo these events are not implemented yet
    WINDOW_FOCUS = bit(4),
    WINDOW_LOST_FOCUS = bit(5),
    WINDOW_MOVED = bit(6),

    UI_WINDOW_CLOSE = bit(7),

    // application events
    APP_TICK = bit(8),
    APP_UPDATE = bit(9),
    APP_RENDER = bit(10),
    APP_LAYER = bit(11),
    SCRIPT_RELOAD = bit(12),
    PROJECT_DIR_UPDATE = bit(13),

    // input events
    KEY_PRESSED = bit(14),
    KEY_RELEASED = bit(15),
    KEY_TYPED = bit(16),
    KEY_HELD = bit(17),
    MOUSE_BUTTON_PRESSED = bit(18),
    MOUSE_BUTTON_RELEASED = bit(19),
    MOUSE_BUTTON_HELD = bit(20),
    MOUSE_MOVD = bit(21),
    MOUSE_SCROLLED = bit(22),

    // scene events
    SCENE_LOAD = bit(23),
    SCENE_START = bit(24),
    SCENE_STOP = bit(25),
    SCENE_UNLOAD = bit(26),

    // editor events
    EDITOR_SCENE_PLAY = bit(27),
    EDITOR_SCENE_PAUSE = bit(28),
    EDITOR_SCENE_STOP = bit(29),

    // core events
    SHUTDOWN = bit(30),
    ENGINE_LAYER = bit(31)
  };

  enum EventCategory : uint64_t {
    NONE = 0,
    APPLICATION_EVENT = bit(0),
    WINDOW_EVENT = bit(1),
    INPUT_EVENT = bit(2),
    KEYBOARD_EVENT = bit(3),
    MOUSE_EVNT = bit(4),
    MOUSE_BUTTON_EVENT = bit(5),
    SCENE_EVENT = bit(6),
    EDITOR_EVENT = bit(7),
    SHUTDOWN_EVENT = bit(8),
    CORE_EVENT = bit(9),
    UI_EVENT = bit(10),
  };

#define EVENT_TYPE(type)                                              \
  static EventType GetStaticType() { return EventType::type; }        \
  virtual EventType Type() const override { return GetStaticType(); } \
  virtual std::string EventName() const override { return #type; }    \
  virtual uint32_t Size() const override { return sizeof(*this); }

#define EVENT_CATEGORY(category) \
  virtual uint32_t CategoryFlags() const override { return category; }

  class Event {
   public:
    Event() {}
    virtual ~Event() {}

    virtual EventType Type() const { return EventType::EMPTY_EVNT; }
    virtual std::string EventName() const { return "EmptyEvent"; }
    virtual uint32_t CategoryFlags() const { return EventCategory::NONE; }
    virtual uint32_t Size() const { return sizeof(Event); };

    virtual std::string ToString() const { return EventName(); }
    inline bool InCategory(EventCategory category) const { return CategoryFlags() & category; }

    bool handled = false;
  };

  template <typename T>
  T* Cast(Event* event) {
    if (event->Type() == T::GetStaticType()) {
      return static_cast<T*>(event);
    }

    return nullptr;
  }

  inline std::ostream& operator<<(std::ostream& os, const Event& e) {
    return os << e.EventName();
  }

  template <typename T>
  concept event_t = std::derived_from<T, Event>;

}  // namespace other

#endif  // !OTHER_ENGINE_EVENT_HPP

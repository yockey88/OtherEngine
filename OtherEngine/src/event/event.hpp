/**
 * \file event/event.hpp
 */
#ifndef OTHER_ENGINE_EVENT_HPP
#define OTHER_ENGINE_EVENT_HPP

#include <string>
#include <type_traits>

#include "core/defines.hpp"
#include "core/logger.hpp"

namespace other {

  enum EventType : uint64_t {
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

#define EVENT_TYPE(type)                                       \
  static EventType GetStaticType() { return EventType::type; } \
  EventType Type() const { return GetStaticType(); }           \
  std::string EventName() const { return #type; }              \
  uint32_t Size() const { return sizeof(*this); }

#define EVENT_CATEGORY(category) \
  uint32_t CategoryFlags() const { return category; }

  template <typename E>
  concept Event =
    std::is_trivially_copyable_v<E> &&
    requires(E e) {
      { E::GetStaticType() } -> std::same_as<other::EventType>;
      { e.Type() } -> std::same_as<other::EventType>;
      { e.EventName() } -> std::same_as<std::string>;
      { e.Size() } -> std::same_as<uint32_t>;
    };

  struct EventHandle {
    void* ptr;
    EventType type;
    size_t wrapped_event_size;

    /// necessary for concept but SHOULD NOT BE USED!!!!
    static EventType GetStaticType() { return EMPTY_EVNT; }
    EventType Type() const { return type; }
    std::string EventName() const { return "EventHandle"; }
    uint32_t Size() const { return sizeof(*this); }
    EVENT_CATEGORY(NONE);
  };

  static_assert(Event<EventHandle>, "EventHandle does not meet the Event concept");

  template <typename E, typename T>
    requires Event<E> && Event<T>
  T* Cast(E* event) {
    OE_ASSERT(event != nullptr, "Event is null");
    if (event->Type() == T::GetStaticType()) {
      return static_cast<T*>(event);
    }

    return nullptr;
  }

  template <typename E>
    requires Event<E>
  E* Cast(void* event) {
    OE_ASSERT(event != nullptr, "Event is null");
    return static_cast<E*>(event);
  }

}  // namespace other

#endif  // !OTHER_ENGINE_EVENT_HPP

/**
 * \file event/event.hpp
 */
#ifndef OTHER_ENGINE_EVENT_HPP
#define OTHER_ENGINE_EVENT_HPP

#include <cstdio>
#include <string>
#include <type_traits>

#include "core/defines.hpp"
#include "core/logger.hpp"

namespace other {

  enum class EventType : uint64_t {
    EMPTY_EVNT = 0,

    // window events
    WINDOW_CLOSE,
    WINDOW_MINIMIZE,
    WINDOW_RESIZE,
    /// \todo these events are not implemented yet
    WINDOW_FOCUS,
    WINDOW_LOST_FOCUS,
    WINDOW_MOVED,

    UI_WINDOW_CLOSE,

    // application events
    APP_TICK,
    APP_UPDATE,
    APP_RENDER,
    APP_LAYER,
    SCRIPT_RELOAD,
    PROJECT_DIR_UPDATE,

    // input events
    KEY_PRESSED,
    KEY_RELEASED,
    KEY_TYPED,
    KEY_HELD,
    MOUSE_BUTTON_PRESSED,
    MOUSE_BUTTON_RELEASED,
    MOUSE_BUTTON_HELD,
    MOUSE_MOVD,
    MOUSE_SCROLLED,

    // scene events
    SCENE_LOAD,
    SCENE_ACTIVATE,
    SCENE_START,
    SCENE_STOP,
    SCENE_UNLOAD,

    // editor events
    EDITOR_SCENE_PLAY,
    EDITOR_SCENE_PAUSE,
    EDITOR_SCENE_STOP,

    // core events
    SHUTDOWN,
    ENGINE_LAYER,

    // filesystem events
    CREATE_DIR,
    DELETE_DIR,
    CREATE_FILE,
    DELETE_FILE,
    MODIFY_FILE,
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
    FILESYSTEM_EVENT = bit(11),
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
    static EventType GetStaticType() { return EventType::EMPTY_EVNT; }
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

/**
 * \file event/core_events.hpp
 */
#ifndef OTHER_ENGINE_CORE_EVENTS_HPP
#define OTHER_ENGINE_CORE_EVENTS_HPP

#include <sstream>

#include "core/defines.hpp"
#include "core/uuid.hpp"

#include "event/event.hpp"

namespace other {

  struct ShutdownEvent {
    ExitCode exit_code = ExitCode::SUCCESS;

    std::string ToString() const {
      std::stringstream ss;
      ss << "ShutdownEvent: " << exit_code;
      return ss.str();
    }

    EVENT_TYPE(SHUTDOWN);
    EVENT_CATEGORY(APPLICATION_EVENT | SHUTDOWN_EVENT | CORE_EVENT);
  };
  static_assert(Event<ShutdownEvent>, "ShutdownEvent does not meet the Event concept");

  enum class LayerEventType : uint8_t {
    LAYER_PUSH,
    LAYER_POP,
    OVERLAY_PUSH,
    OVERLAY_POP
  };

#define LAYER_EVENT()                                                         \
  LayerEventType type;                                                        \
  uint64_t layer_id;                                                          \
  bool IsPush() const { return type == LayerEventType::LAYER_PUSH; }          \
  bool IsPop() const { return type == LayerEventType::LAYER_POP; }            \
  bool IsOverlayPush() const { return type == LayerEventType::OVERLAY_PUSH; } \
  bool IsOverlayPop() const { return type == LayerEventType::OVERLAY_POP; }   \
  UUID LayerID() const { return layer_id; }                                   \
  EVENT_CATEGORY(APPLICATION_EVENT | CORE_EVENT);

  struct EngineLayerEvent {
    LAYER_EVENT();
    EVENT_TYPE(ENGINE_LAYER);

    std::string ToString() const {
      std::stringstream ss;
      ss << "EngineLayerEvent: "
         << (type == LayerEventType::LAYER_PUSH || type == LayerEventType::OVERLAY_PUSH ?
               "Push" :
               "Pop");
      return ss.str();
    }
  };
  static_assert(Event<EngineLayerEvent>, "EngineLayerEvent does not meet the Event concept");

#define FILESYSTEM_EVENT() \
  EVENT_CATEGORY(FILESYSTEM_EVENT | CORE_EVENT);

  struct CreateDirectoryEvent {
    FILESYSTEM_EVENT();
    EVENT_TYPE(CREATE_DIR);

    uint64_t handle = 0;

    std::string ToString() const {
      std::stringstream ss;
      ss << "FilesystemEvent";
      return ss.str();
    }
  };
  static_assert(Event<CreateDirectoryEvent>, "CreateDirectoryEvent does not meet the Event concept");

  struct DeleteDirectoryEvent {
    FILESYSTEM_EVENT();
    EVENT_TYPE(DELETE_DIR);

    uint64_t handle = 0;

    std::string ToString() const {
      std::stringstream ss;
      ss << "FilesystemEvent";
      return ss.str();
    }
  };
  static_assert(Event<DeleteDirectoryEvent>, "DeleteDirectoryEvent does not meet the Event concept");

  struct CreateFileEvent {
    FILESYSTEM_EVENT();
    EVENT_TYPE(CREATE_FILE);

    uint64_t handle = 0;

    std::string ToString() const {
      std::stringstream ss;
      ss << "FilesystemEvent";
      return ss.str();
    }
  };
  static_assert(Event<CreateFileEvent>, "CreateFileEvent does not meet the Event concept");

  struct DeleteFileEvent {
    FILESYSTEM_EVENT();
    EVENT_TYPE(DELETE_FILE);

    uint64_t handle = 0;

    std::string ToString() const {
      std::stringstream ss;
      ss << "FilesystemEvent";
      return ss.str();
    }
  };
  static_assert(Event<DeleteFileEvent>, "DeleteFileEvent does not meet the Event concept");

  struct ModifyFileEvent {
    FILESYSTEM_EVENT();
    EVENT_TYPE(MODIFY_FILE);

    uint64_t handle = 0;

    std::string ToString() const {
      std::stringstream ss;
      ss << "FilesystemEvent";
      return ss.str();
    }
  };
  static_assert(Event<ModifyFileEvent>, "ModifyFileEvent does not meet the Event concept");

}  // namespace other

#endif  // !OTHER_ENGINE_CORE_EVENTS_HPP

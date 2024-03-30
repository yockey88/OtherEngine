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

  class ShutdownEvent : public Event {
    public:
      ShutdownEvent(ExitCode exit_code) 
        : exit_code(exit_code) {}
      ExitCode GetExitCode() const { return exit_code; }

      virtual std::string ToString() const override {
        std::stringstream ss;
        ss << "ShutdownEvent: " << exit_code;
        return ss.str();
      }

      EVENT_TYPE(SHUTDOWN);
      EVENT_CATEGORY(APPLICATION_EVENT | SHUTDOWN_EVENT | CORE_EVENT);

    protected:
      ExitCode exit_code = ExitCode::SUCCESS;
  };
  
  enum class LayerEventType {
    LAYER_PUSH ,
    LAYER_POP ,
    OVERLAY_PUSH ,
    OVERLAY_POP
  };

  class LayerEvent : public Event {
    public:
      LayerEvent(LayerEventType type , UUID layer_id , const std::string& layer_name)
        : type(type) , layer_id(layer_id) , layer_name(layer_name) {}

      bool IsPush() const { return type == LayerEventType::LAYER_PUSH; }
      bool IsPop() const { return type == LayerEventType::LAYER_POP; }
      bool IsOverlayPush() const { return type == LayerEventType::OVERLAY_PUSH; }
      bool IsOverlayPop() const { return type == LayerEventType::OVERLAY_POP; }
      const UUID LayerID() const { return layer_id; }
      const std::string LayerName() const { return layer_name; }

      EVENT_CATEGORY(APPLICATION_EVENT | CORE_EVENT);

    protected:
      LayerEventType type;
      const UUID layer_id;
      const std::string layer_name;
  };

  class EngineLayerEvent : public LayerEvent {
    public:
      EngineLayerEvent(LayerEventType type , UUID layer_id , const std::string& layer_name)
        : LayerEvent(type , layer_id , layer_name) {}

      virtual std::string ToString() const override {
        std::stringstream ss;
        ss << "EngineLayerEvent: " 
           << (type == LayerEventType::LAYER_PUSH || type == LayerEventType::OVERLAY_PUSH ? 
                "Push" : "Pop") 
           << " " << layer_name;
        return ss.str();
      }

      EVENT_TYPE(ENGINE_LAYER);
  };

} // namespace other

#endif // !OTHER_ENGINE_CORE_EVENTS_HPP

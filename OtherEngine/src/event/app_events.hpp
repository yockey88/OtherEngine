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

  class AppLayerEvent : public LayerEvent {
    public:
      AppLayerEvent(LayerEventType type , UUID layer_id , const std::string& layer_name)
        : LayerEvent(type , layer_id , layer_name) {}

      virtual std::string ToString() const override {
        std::stringstream ss;
        ss << "AppLayerEvent: " 
           << (IsPush() ? 
                "Push" : "Pop") 
           << " " << LayerName();
        return ss.str();
      }

      EVENT_TYPE(APP_LAYER);
  };
  
  class ScriptReloadEvent : public Event {
    public:
      ScriptReloadEvent()
        : Event() {}

      virtual std::string ToString() const override {
        return "ScriptReloadEvent";
      }

      EVENT_CATEGORY(APPLICATION_EVENT);
      EVENT_TYPE(SCRIPT_RELOAD);
  };


  class ProjectDirectoryUpdateEvent : public Event {
    public: 
      ProjectDirectoryUpdateEvent(ProjectDirectoryType type)
        : Event() , dir_type(type) {}

      virtual std::string ToString() const override {
        return "ProjectDirectoryUpdateEvent";
      }

      ProjectDirectoryType dir_type;

      EVENT_CATEGORY(APPLICATION_EVENT);
      EVENT_TYPE(PROJECT_DIR_UPDATE);
  };

} // namespace other

#endif // !OTHER_ENGINE_APP_EVENTS_HPP

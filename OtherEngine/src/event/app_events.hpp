/**
 * \file event/app_events.hpp
 */
#ifndef OTHER_ENGINE_APP_EVENTS_HPP
#define OTHER_ENGINE_APP_EVENTS_HPP

#include <sstream>

#include "event/core_events.hpp"

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

} // namespace other

#endif // !OTHER_ENGINE_APP_EVENTS_HPP

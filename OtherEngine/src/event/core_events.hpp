/**
 * \file event/core_events.hpp
*/
#ifndef OTHER_ENGINE_CORE_EVENTS_HPP
#define OTHER_ENGINE_CORE_EVENTS_HPP

#include <sstream>

#include "core/defines.hpp"
#include "event/event.hpp"

namespace other {

  class ShutdownEvent : public Event {
    protected:
      ExitCode exit_code = ExitCode::SUCCESS;

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
      EVENT_CATEGORY(APPLICATION_EVENT | SHUTDOWN_EVENT);
  };

} // namespace other

#endif // !OTHER_ENGINE_CORE_EVENTS_HPP

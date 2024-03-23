/**
 * \file event\event_handler.hpp
*/
#ifndef OTHER_ENGINE_EVENT_HANDLER_HPP
#define OTHER_ENGINE_EVENT_HANDLER_HPP

#include <functional>

#include "event/event.hpp"

namespace other {

  template <event_t T>
  using Handler = std::function<bool(T&)>;

  class EventHandler {
    public:
      EventHandler(Event& event)
        : event(event) {}

      template <event_t T>
      bool Handle(Handler<T> handler) {
        if (event.Type() == T::GetStaticType()) {
          event.handled = handler(*(T*)&event);

          return true;
        }

        return false;
      }

    private:
      Event& event;
  };

} // namespace other

#endif // !OTHER_ENGINE_EVENT_HANDLER_HPP

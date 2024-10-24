/**
 * \file event\event_handler.hpp
 */
#ifndef OTHER_ENGINE_EVENT_HANDLER_HPP
#define OTHER_ENGINE_EVENT_HANDLER_HPP

#include <functional>

#include "core/ref_counted.hpp"

#include "event/event.hpp"

namespace other {

  template <event_t E>
  using Handler = std::function<bool(E&)>;

  template <event_t E>
  struct DispatchInvoker;

  class EventHandler {
   public:
    EventHandler(Event* event)
        : event(event) {}

    template <event_t E>
    bool Handle(Handler<E> handler) {
      E* e = Cast<E>(event);
      if (e == nullptr) {
        return false;
      }

      event->handled = handler(*e);
      return event->handled;
    }

    template <event_t E>
    bool Handle(DispatchInvoker<E>& handler) {
      for (auto& h : handler.handlers) {
        if (Handle(h)) {
          return true;
        }
      }
      return event->handled;
    }

   private:
    Event* event;
  };

  struct Dispatcher : public RefCounted {
    virtual ~Dispatcher() = default;
    virtual EventType Type() const = 0;

    bool Dispatch(Event* event) {
      if (event->Type() != Type()) {
        return false;
      }

      return DispatchEvent(event);
    }

   protected:
    virtual bool DispatchEvent(Event* event) = 0;
  };

  template <event_t E>
  struct DispatchInvoker : public Dispatcher {
    DispatchInvoker(const std::vector<Handler<E>>& event_handlers)
        : handlers(event_handlers) {}

    virtual EventType Type() const override {
      return E::GetStaticType();
    }

    std::vector<Handler<E>> handlers;

   protected:
    virtual bool DispatchEvent(Event* event) override {
      EventHandler handler(event);
      return handler.Handle<E>(*this);
    }
  };

}  // namespace other

#endif  // !OTHER_ENGINE_EVENT_HANDLER_HPP

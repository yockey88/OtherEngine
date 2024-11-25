/**
 * \file event\event_handler.hpp
 */
#ifndef OTHER_ENGINE_EVENT_HANDLER_HPP
#define OTHER_ENGINE_EVENT_HANDLER_HPP

#include <functional>

#include "core/ref_counted.hpp"

#include "event/event.hpp"

namespace other {

  template <typename E>
    requires Event<E>
  using Handler = std::function<bool(E&)>;

  template <typename E>
    requires Event<E>
  struct DispatchInvoker;

  struct EventHandler {
    EventHandler(void* event)
        : event(event) {}

    template <typename E>
      requires Event<E>
    bool Handle(Handler<E> handler) {
      OE_ASSERT(event != nullptr, "Event is null");

      E* e = Cast<E>(event);
      if (e == nullptr) {
        return false;
      }

      return handler(*e);
    }

    template <typename E>
      requires Event<E>
    bool Handle(DispatchInvoker<E>& handler) {
      OE_ASSERT(event != nullptr, "Event is null");
      for (auto& h : handler.handlers) {
        if (Handle(h)) {
          return true;
        }
      }
      return false;
    }

   private:
    void* event;
  };

  struct Dispatcher : public RefCounted {
    virtual ~Dispatcher() = default;
    virtual EventType Type() const = 0;

    bool Dispatch(EventHandle& event) {
      OE_ASSERT(event.ptr != nullptr, "Event is null");
      return event.Type() == Type() && DispatchEvent(event.ptr);
    }

   protected:
    virtual bool DispatchEvent(void* event) = 0;
  };

  template <typename E>
    requires Event<E>
  struct DispatchInvoker : public Dispatcher {
    DispatchInvoker(const std::vector<Handler<E>>& event_handlers)
        : handlers(event_handlers) {}

    virtual EventType Type() const override {
      return E::GetStaticType();
    }

    std::vector<Handler<E>> handlers;

   protected:
    virtual bool DispatchEvent(void* event) override {
      OE_ASSERT(event != nullptr, "Event is null");
      EventHandler handler(event);
      return handler.Handle<E>(*this);
    }
  };

}  // namespace other

#endif  // !OTHER_ENGINE_EVENT_HANDLER_HPP

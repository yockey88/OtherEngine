/**
 * \file core/state.hpp
 **/
#ifndef OTHER_ENGINE_STATE_HPP
#define OTHER_ENGINE_STATE_HPP

#include <concepts>

#include "core/ref.hpp"
#include "core/ref_counted.hpp"

namespace other {

  struct StateEvent : public RefCounted {
    virtual ~StateEvent() override {}
  };

  template <typename ET>
  concept StateEventType = std::derived_from<ET, StateEvent>;

  struct State : public RefCounted {
    virtual ~State() override {}
    virtual Ref<State> ProcessEvent(const Ref<StateEvent>& event) = 0;

    void Step();

   protected:
    virtual void OnStep() {}
  };

  template <typename ST, typename ET>
    requires StateEventType<ET>
  struct StateImpl : public State {
    virtual ~StateImpl() override {}
    virtual Ref<State> ProcessEvent(const Ref<StateEvent>& event) override {
      Ref<ET> cast_event = event;
      OE_ASSERT(cast_event != nullptr, "Invalid event type");

      if (cast_event != nullptr) {
        return HandleEvent(cast_event);
      }

      return nullptr;
    }

    virtual Ref<ST> HandleEvent(const Ref<ET>& event) = 0;
  };

  template <typename ST>
  concept StateType = std::derived_from<ST, State>;

  template <typename ST, typename ET>
  concept StateImplType = StateType<ST> && std::derived_from<ST, StateImpl<ST, ET>>;

  template <typename ST, typename ET>
  constexpr inline bool kHasStateImpl = StateImplType<ST, ET> && StateEventType<ET>;

  template <typename ST, typename ET>
  concept CompatibleStateSets = kHasStateImpl<ST, ET>;

  template <typename ST, typename ET>
  concept StateMachineTypes =
    CompatibleStateSets<ST, ET> &&
    requires(Ref<ST> state_type, Ref<ET> event_type) {
      { state_type->ProcessEvent(std::declval<const Ref<StateEvent>&>()) } -> std::same_as<Ref<State>>;
      { state_type->HandleEvent(event_type) } -> std::same_as<Ref<ST>>;
    };

  class StateMachine : public RefCounted {
   public:
    StateMachine() = default;
    virtual ~StateMachine() override {}

    virtual void DispatchEvent(const Ref<StateEvent>& event) = 0;
    virtual Ref<State> CurrentState() = 0;

    void Step();

   protected:
    virtual void OnStep() {}
  };

  template <typename ST, typename ET>
    requires StateMachineTypes<ST, ET>
  class StateMachineImpl : public StateMachine {
   public:
    StateMachineImpl(const Ref<ST>& first_state)
        : current_state(first_state) {}
    virtual ~StateMachineImpl() override {}

    virtual void DispatchEvent(const Ref<StateEvent>& event) override {
      OE_ASSERT(current_state != nullptr, "Invalid state");
      OE_ASSERT(event != nullptr, "Invalid event");

      Ref<ST> new_state = current_state->ProcessEvent(event);
      if (new_state != nullptr) {
        current_state = new_state;
      }
    }

    virtual Ref<State> CurrentState() override {
      OE_ASSERT(current_state != nullptr, "Invalid state");
      return current_state;
    }

    void OnStep() override {
      OE_ASSERT(current_state != nullptr, "Invalid state");
      current_state->Step();
    }

   protected:
    Ref<ST> current_state = nullptr;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_STATE_HPP
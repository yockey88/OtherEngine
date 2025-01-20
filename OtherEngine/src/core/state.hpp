/**
 * \file core/state.hpp
 **/
#ifndef OTHER_ENGINE_STATE_HPP
#define OTHER_ENGINE_STATE_HPP

#include <concepts>

#include "core/logger.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"

namespace other {

  struct State : public RefCounted {
    virtual ~State() override {}

    void Attach();
    void Step();
    void Detach();

   protected:
    virtual void OnAttach() {}
    virtual void OnStep() {}
    virtual void OnDetach() {}
  };

  template <typename ET>
  concept StateEventType = std::is_enum_v<ET>;

  template <typename ST, typename ET>
    requires StateEventType<ET>
  struct StateImpl : public State {
    virtual ~StateImpl() override {}
    Ref<State> ProcessEvent(const ET event) {
      return HandleEvent(event);
    }

    virtual Ref<ST> HandleEvent(const ET event) = 0;
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
    requires(Ref<ST> state_type, const ET event_type) {
      { state_type->ProcessEvent(std::declval<const ET>()) } -> std::same_as<Ref<State>>;
      { state_type->HandleEvent(event_type) } -> std::same_as<Ref<ST>>;
    };

  template <typename ST, typename ET>
    requires StateMachineTypes<ST, ET>
  class StateMachine : public RefCounted {
   public:
    StateMachine(const Ref<ST>& first_state)
        : current_state(first_state) {
      OE_ASSERT(current_state != nullptr, "Invalid state");
      current_state->Attach();
    }
    virtual ~StateMachine() {}

    void HandleEvent(const ET event) {
      PROFILE_SECTION("StateMachine--HandleEvent");
      OE_ASSERT(current_state != nullptr, "Invalid state");

      Ref<ST> new_state = current_state->ProcessEvent(event);
      if (new_state != nullptr) {
        current_state->Detach();
        new_state->Attach();
        current_state = new_state;
      }
      /// nullptr means we have accepted final state,
      ///   detach and clear our state,
      ///   HandleEvent should NOT be called again
      else {
        current_state->Detach();
        current_state = nullptr;
      }
    }

    bool IsFinished() {
      return current_state == nullptr;
    }

    bool IsError() {
      return false;
    }

    Ref<ST> CurrentState() {
      OE_ASSERT(current_state != nullptr, "Invalid state");
      return current_state;
    }

    void Step() {
      OE_ASSERT(current_state != nullptr, "Invalid state");
      current_state->Step();
    }

   protected:
    Ref<ST> current_state = nullptr;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_STATE_HPP

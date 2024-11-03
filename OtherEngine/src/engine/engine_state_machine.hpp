/**
 * \file application/app_state_machine.hpp
 **/
#ifndef OTHER_ENGINE_APP_STATE_MACHINE_HPP
#define OTHER_ENGINE_APP_STATE_MACHINE_HPP

#include <magic_enum/magic_enum.hpp>

#include "core/state.hpp"

namespace other {

  class Engine;

  enum class EngineStateEvent : uint64_t {
    NO_EVENT = 0,

    ENGINE_LOAD_FINISHED = bit(1),
    ENGINE_UNLOAD_FINISHED = bit(2),
    ENGINE_SHUTDOWN = bit(3),

    APP_ATTACHED = bit(4),
    APP_DETACHED = bit(5),

    SCENE_LOADED = bit(6),
    SCENE_UNLOADED = bit(7),
  };

}  // namespace other

template <>
struct fmt::formatter<other::EngineStateEvent> : public fmt::formatter<std::string_view> {
  auto format(other::EngineStateEvent e, fmt::format_context& ctx) {
    std::string_view enum_name = magic_enum::enum_name(e);
    return fmt::formatter<std::string_view>::format(enum_name, ctx);
  }
};

namespace other {

  enum class EngineStateTypes : uint64_t {
    IDLE = 0,
    ENGINE_IDLE = bit(1),
    APP_IDLE = bit(2),
    SCENE_IDLE = bit(3),
  };

  struct EngineState : public StateImpl<EngineState, EngineStateEvent> {
    EngineState(Engine* engine, EngineStateTypes type)
        : type(type), engine(engine) {
      OE_ASSERT(engine != nullptr, "Invalid engine pointer");
    }
    virtual ~EngineState() override {}
    EngineStateTypes type;

    Engine* engine = nullptr;
  };

  /// core engine states

  struct EngineLaunching : public EngineState {
    EngineLaunching(Engine* engine)
        : EngineState(engine, EngineStateTypes::IDLE) {}
    virtual ~EngineLaunching() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    virtual void OnAttach() override;
    virtual void OnStep() override;
    virtual void OnDetach() override;
  };

  struct EngineIdle : public EngineState {
    EngineIdle(Engine* engine)
        : EngineState(engine, EngineStateTypes::ENGINE_IDLE) {}
    virtual ~EngineIdle() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    virtual void OnAttach() override;
    virtual void OnStep() override;
    virtual void OnDetach() override;
  };

  struct EngineShutdown : public EngineState {
    EngineShutdown(Engine* engine)
        : EngineState(engine, EngineStateTypes::IDLE) {}
    virtual ~EngineShutdown() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    virtual void OnAttach() override;
    virtual void OnStep() override;
    virtual void OnDetach() override;
  };

  /// application states

  struct AppIdle : public EngineState {
    AppIdle(Engine* engine)
        : EngineState(engine, EngineStateTypes::APP_IDLE) {}
    virtual ~AppIdle() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    virtual void OnAttach() override;
    virtual void OnStep() override;
    virtual void OnDetach() override;

    bool immediate_scene_load = false;
  };

  struct SceneIdle : public EngineState {
    SceneIdle(Engine* engine)
        : EngineState(engine, EngineStateTypes::SCENE_IDLE) {}
    virtual ~SceneIdle() override {}
    virtual Ref<EngineState> HandleEvent(const EngineStateEvent event) override;

    virtual void OnAttach() override;
    virtual void OnStep() override;
    virtual void OnDetach() override;
  };

  class EngineStateMachine : public StateMachine<EngineState, EngineStateEvent> {
   public:
    EngineStateMachine(Engine* engine)
        : StateMachine<EngineState, EngineStateEvent>(NewRef<EngineLaunching>(engine)), engine(engine) {}
    virtual ~EngineStateMachine() override {}

    Engine* engine = nullptr;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_APP_STATE_MACHINE_HPP
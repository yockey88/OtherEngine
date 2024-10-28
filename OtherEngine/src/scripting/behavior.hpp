/**
 * \file scripting/behavior.hpp
 **/
#ifndef OTHER_ENGINE_BEHAVIOR_HPP
#define OTHER_ENGINE_BEHAVIOR_HPP

#include "core/defines.hpp"
#include "core/ref.hpp"
#include "core/uuid.hpp"

#include "scripting/script_object.hpp"

namespace other {

  enum class BehaviorType {
    EMPTY_BEHAVIOR,
    GENERIC_BEHAVIOR,
    UI_BEHAVIOR,
    // PHYSICS_BEHAVIOR,
    SCRIPT_BEHAVIOR,
    // ANIMATION_BEHAVIOR,
    // AUDIO_BEHAVIOR,
    // RENDER_BEHAVIOR,
    // SCENE_BEHAVIOR,
    // INPUT_BEHAVIOR,
    // NETWORK_BEHAVIOR,
    // AI_BEHAVIOR,
    // CUSTOM_BEHAVIOR,

    NUM_BEHAVIORS,
    INVALID_BEHAVIOR = -1,
  };

  static constexpr size_t kNumBehaviors = ValOf(BehaviorType::NUM_BEHAVIORS);
  static std::array<std::string_view, kNumBehaviors> kBehaviorConfigFileNames = {
    "generic",
    "ui",
    // "physics",
    "script",
    // "animation",
    // "audio",
    // "render",
    // "scene",
    // "input",
  };

  struct Behavior {
    BehaviorType type = BehaviorType::EMPTY_BEHAVIOR;
    Ref<ScriptObject> script_object = nullptr;

    UUID handle = 0;

    Behavior() = default;
    Behavior(BehaviorType t, Ref<ScriptObject> obj)
        : type(t), script_object(obj) {}
  };

}  // namespace other

#endif  // !OTHER_ENGINE_BEHAVIOR_HPP
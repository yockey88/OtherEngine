/**
 * \file editor/saves.hpp
 **/
#include "editor/saves.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/tag.hpp"

namespace other {

  void StateStack::RestoreState(Ref<Scene> &scene, const StateCapture &capture) {
    for (auto& [id , transform] : capture.transforms) {
      Entity* ent = scene->GetEntity(id);
      ent->GetComponent<Transform>() = transform;
    }
  }

  StateCapture StateStack::RecordState(Ref<Scene>& scene) {
    StateCapture capture;

    auto& registry = scene->Registry();

    registry.view<Tag , Transform>().each([&capture](const Tag& tag , const Transform& transform) {
      capture.transforms[tag.id] = transform;
    });

    return capture;
  }

} // namespace other

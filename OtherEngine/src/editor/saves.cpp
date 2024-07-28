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
      auto& t = ent->GetComponent<Transform>();
      t.position = transform.position;
      t.scale = transform.scale;
      t.erotation = transform.erotation;
      t.qrotation = transform.qrotation;
      t.CalcMatrix();
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

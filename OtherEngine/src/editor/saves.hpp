/**
 * \file editor/saves.hpp
 **/
#ifndef OTHER_ENGINE_SAVES_HPP
#define OTHER_ENGINE_SAVES_HPP

#include <entt/entt.hpp>
#include <entt/entity/snapshot.hpp>

#include "scene/scene.hpp"

#include "ecs/components/transform.hpp"

namespace other {

  struct StateCapture {
    UUID scene_id;

    /// capture entities
    /// transforms
    std::map<UUID , Transform> transforms;

  };

  class StateStack {
    public:
      static void RestoreState(Ref<Scene>& scene , const StateCapture& capture);
      static StateCapture RecordState(Ref<Scene>& scene);
  };


} // namespace other

#endif // !OTHER_ENGINE_SAVES_HPP

/**
 * \file scene/scene_capture.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_CAPTURE_HPP
#define OTHER_ENGINE_SCENE_CAPTURE_HPP

#include <stack>

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

#include "core/ref.hpp"
#include "core/uuid.hpp"

#include "ecs/entity.hpp"

namespace other {

  class Scene;

  struct SceneCapture : public RefCounted {
    std::map<UUID, entt::entity> entity_map;
    entt::registry registry;
  };

  class CaptureStack {
   public:
    UUID scene_id;

    bool PopCapture(Scene* scene);
    void PushCapture(Scene* scene);

   private:
    std::stack<Ref<SceneCapture>> capture_stack;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_SCENE_CAPTURE_HPP

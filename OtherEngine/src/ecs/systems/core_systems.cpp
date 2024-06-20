/**
 * \file ecs/systems/core_systems.cpp
 **/
#include "ecs/systems/core_systems.hpp"

#include "ecs/component.hpp"
#include "ecs/components/serialization_data.hpp"
#include "ecs/entity.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/camera.hpp"

#include "rendering/perspective_camera.hpp"

namespace other {

  CORE_SYSTEM(OnConstructEntity) {
    Entity e(context , entt);
    auto& tag = e.AddComponent<Tag>();
    tag.id = 0;
    tag.name = "[ Blank Entity ]";
    
    auto& transform = e.AddComponent<Transform>();
    transform.position = { 0.f , 0.f , 0.f };
    transform.erotation = { 0.f , 0.f , 0.f };
    transform.scale = { 1.f , 1.f , 1.f };

    /* auto& relationship = */ e.AddComponent<Relationship>();
    
    /* auto& serialization_data = */ e.AddComponent<SerializationData>();
  }
  
  CORE_SYSTEM(OnDestroyEntity) {
    context.view<Script>().each([](Script& script) {
      for (auto& [id , script] : script.scripts) {
        if (script->IsInitialized()) {
          script->Shutdown();
        }
      }
      script.scripts.clear();
    });
  }

  CORE_SYSTEM(OnCameraAddition) {
    Entity ent(context , entt);
    auto& camera = ent.GetComponent<Camera>();
    camera.camera = Ref<PerspectiveCamera>::Create();
  }

} // namespace other

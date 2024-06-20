/**
 * \file ecs/entity.cpp
 **/
#include "ecs/entity.hpp"

#include "scene/scene.hpp"
#include "scene/octree.hpp"

#include "ecs/components/tag.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/camera.hpp"

namespace other {

  Entity::Entity(Ref<Scene>& ctx , UUID uuid , const std::string& name)
      : registry(ctx->registry) , uuid(uuid) , name(name) {
    context = ctx;
    handle = context->registry.create();

    auto& tag = GetComponent<Tag>();
    tag.id = uuid;
    tag.name = name;
  }
      
  Entity::Entity(Scene* ctx , UUID uuid , const std::string& name) 
      : registry(ctx->registry) , uuid(uuid) , name(name) {
    context = Ref<Scene>(ctx);
    handle = registry.create();

    auto& tag = GetComponent<Tag>();
    tag.id = uuid;
    tag.name = name;
  }

} // namespace other

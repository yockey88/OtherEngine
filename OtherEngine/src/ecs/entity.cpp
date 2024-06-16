/**
 * \file ecs/entity.cpp
 **/
#include "ecs/entity.hpp"

#include "scene/scene.hpp"
#include "scene/octree.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/mesh.hpp"

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
      
  const std::vector<uint32_t> Entity::GetComponentIndices() const {
    /// we don't serialize tag so we don't add its index (0) into the array and every entity has
    ///  an implicit transform and relationship component (1 , 2) so we don't add those indices either
    std::vector<uint32_t> res; 

    if (HasComponent<Mesh>()) {
      res.push_back(kMeshIndex);
    }
    
    if (HasComponent<Script>()) {
      res.push_back(kScriptIndex);
    }

    return res;
  }

} // namespace other

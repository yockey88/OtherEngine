/**
 * \file scene/scene.cpp
 **/
#include "scene/scene.hpp"

#include "core/rand.hpp"
#include "ecs/entity.hpp"

namespace other {

  void Scene::AddEntity(const std::string& name) {
    auto handle = registry.create();
    Entity entity(&registry , handle, Random::GenerateUUID(), name);
  }

} // namespace other

/**
 * \file yockcraft.cpp
 **/
#include "yockcraft.hpp"

#include <ecs/entity.hpp>
#include <ecs/components/light_source.hpp>
#include <ecs/components/mesh.hpp>
#include <rendering/material.hpp>

#include "rendering/model_factory.hpp"

void YockCraft::OnAttach() {
}
    
void YockCraft::OnSceneLoad(const SceneMetadata* scene_md) {
  if (scene_md->name == "overworld") {
    auto* sun = scene_md->scene->GetEntity(FNV("sun"));
    auto& light = sun->GetComponent<LightSource>();
    light.direction_light = {
      .direction = { 0.f , -1.f , 0.f , 1.f } ,
      .color = { 0.22f , 0.22f , 0.11f , 1.f } ,
    };

    // auto* terrain = scene_md->scene->GetEntity(FNV("terrain"));
    // auto& terrain_mesh = terrain->AddComponent<StaticMesh>();
    // terrain_mesh.handle = ModelFactory::CreateBox({ 1.f , 1.f , 1.f });
    // terrain_mesh.is_primitive = true;
    // terrain_mesh.primitive_id = 3;
    // terrain_mesh.material = {
    //   .color = { 0.2f , 0.3f , 0.4f , 1.f } ,
    //   .shininess = 32.f
    // };
  }
}

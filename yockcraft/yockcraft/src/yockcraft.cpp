/**
 * \file yockcraft.cpp
 **/
#include "yockcraft.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/light_source.hpp"

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
  }
}

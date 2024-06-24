/**
 * \file ecs/components/camera.cpp
 **/
#include "ecs/components/camera.hpp"

#include "core/logger.hpp"
#include "core/config_keys.hpp"
#include "scene/scene.hpp"
#include "ecs/entity.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/orthographic_camera.hpp"

namespace other {

  void CameraSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {
    const auto& camera = entity->GetComponent<Camera>();

    SerializeComponentSection(stream , entity , "camera");
    stream << "type = " << camera.camera->GetCameraTypeString() << "\n";
    SerializeVec3(stream , "position" , camera.camera->Position());
    SerializeVec3(stream , "direction" , camera.camera->Direction());
    SerializeVec3(stream , "up" , camera.camera->Up());
    SerializeVec3(stream , "world-up" , camera.camera->WorldUp());
    SerializeVec3(stream , "right" , camera.camera->Right());
    SerializeVec3(stream , "rotation" , camera.camera->Orientation());
    SerializeVec2(stream , "viewport" , camera.camera->Viewport());
    SerializeVec2(stream , "clip" , camera.camera->Clip());
    SerializeValue(stream , "speed" , camera.camera->Speed());
    SerializeValue(stream , "sensitivity" , camera.camera->Sensitivity());
    SerializeValue(stream , "fov" , camera.camera->FOV());
    SerializeValue(stream , "zoom" , camera.camera->Zoom());
    SerializeValue(stream , "constrain-pitch" , camera.camera->ConstrainPitch());
    SerializeValue(stream , "primary" , camera.camera->IsPrimary());
    stream << "\n";
  }

  void CameraSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr , "Attempting to deserialize a camera into null entity or scene");
    std::string key_value = GetComponentSectionKey(entity->Name() , std::string{ kCameraValue });
    
    auto camera_type = scene_table.Get(key_value , kTypeValue);
    if (camera_type.size() != 1) {
      OE_ERROR("Failed to deserialize camera component into entity {}" , entity->Name());
      return;
    }
    
    auto& camera = entity->AddComponent<Camera>();

    std::string type = camera_type[0];
    std::string uc_type;
    std::transform(type.begin() , type.end() , std::back_inserter(uc_type) , ::toupper);

    switch (FNV(uc_type)) {
      case kPerspectiveCamValueHash: 
        camera.camera = Ref<PerspectiveCamera>::Create();
      break;

      case kOrthographicCamValueHash:
        camera.camera = Ref<OrthographicCamera>::Create();
      break;

      default:
        OE_ERROR("Failed to deserialize camera component into entity {}" , entity->Name());
        entity->RemoveComponent<Camera>();
        return;
    }

    auto pos_value = scene_table.Get(key_value , kPositionValue);
    auto dir_value = scene_table.Get(key_value , kDirectionValue);
    auto up_value = scene_table.Get(key_value , kUpValue);
    auto right_value = scene_table.Get(key_value , kRightValue);
    auto wup_value = scene_table.Get(key_value , kWorldUpValue);

    auto ea_value = scene_table.Get(key_value , kRotationValue);
    auto vp_value = scene_table.Get(key_value , kViewportValue);
    auto clip_value = scene_table.Get(key_value , kClipValue);

    auto speed_value = scene_table.GetVal<float>(key_value , kSpeedValue);
    auto sens_value = scene_table.GetVal<float>(key_value , kSensitivityValue);
    auto fov_value = scene_table.GetVal<float>(key_value , kFovValue);
    auto zoom_value = scene_table.GetVal<float>(key_value , kZoomValue);

    auto cpitch_value = scene_table.GetVal<bool>(key_value , kConstrainPitchValue);
    auto primary_value = scene_table.GetVal<bool>(key_value , kPrimaryValue); 

    if (pos_value.size() > 0) {
      glm::vec3 position = glm::vec3(0.0f , 0.0f , 3.0f);
      DeserializeVec3(pos_value , position);
      camera.camera->SetPosition(position);
    }
    
    if (dir_value.size() > 0) {
      glm::vec3 direction = glm::vec3(0.0f , 0.0f , -1.0f);
      DeserializeVec3(dir_value , direction);
      camera.camera->SetPosition(direction);
    }
    
    if (right_value.size() > 0) {
      glm::vec3 right = glm::vec3(1.0f , 0.0f , 0.0f);
      DeserializeVec3(right_value , right);
      camera.camera->SetPosition(right);
    }

    if (up_value.size() > 0) {
      glm::vec3 up = glm::vec3(0.0f , 1.0f , 0.0f);
      DeserializeVec3(up_value , up);
      camera.camera->SetUp(up);
    }
    
    if (wup_value.size() > 0) {
      glm::vec3 wup = glm::vec3(0.0f , 1.0f , 0.0f);
      DeserializeVec3(wup_value , wup);
      camera.camera->SetWorldUp(wup);
    }

    if (ea_value.size() > 0) {
      glm::vec3 euler_angles = glm::vec3(-90.0f , 0.0f , 0.0f);
      DeserializeVec3(ea_value , euler_angles);
      camera.camera->SetOrientation(euler_angles);
    }

    if (vp_value.size() > 0) {
      glm::ivec2 viewport = glm::ivec2(800 , 600);
      DeserializeVec2(vp_value , viewport);
    }
    
    if (clip_value.size() > 0) {
      glm::vec2 clip = glm::vec2(0.1f , 100.0f);
      DeserializeVec2(clip_value , clip);
      camera.camera->SetClip(clip);
    }

    if (speed_value.has_value()) {
      camera.camera->SetSpeed(speed_value.value());
    }

    if (sens_value.has_value()) {
      camera.camera->SetSensitivity(speed_value.value());
    }

    if (fov_value.has_value()) {
      camera.camera->SetFov(fov_value.value());
    }

    if (zoom_value.has_value()) {
      camera.camera->SetZoom(zoom_value.value());
    }

    if (cpitch_value.has_value()) {
      camera.camera->SetConstrainPitch(cpitch_value.value());
    }

    if (primary_value.has_value()) {
      camera.camera->SetPrimary(primary_value.value());
    }
  }

} // namespace other

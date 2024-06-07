/**
 * \file ecs/component_serializer.hpp
 **/
#ifndef OTHER_ENGINE_COMPONENT_SERIALIZER_HPP
#define OTHER_ENGINE_COMPONENT_SERIALIZER_HPP

#include "core/config.hpp"
#include "core/ref.hpp"

namespace other {

  class Entity;
  class Scene;

  class ComponentSerializer {
    public:
      virtual ~ComponentSerializer() {}

      virtual void Serialize(std::ostream& stream , Entity* owner , const Ref<Scene>& scene) const = 0;
      virtual void Deserialize(Entity* entity , const ConfigTable& scn_table , Ref<Scene>& scene) const = 0;

    protected:
      void SerializeVec2(std::ostream& stream , const std::string_view name , const glm::vec2& vec) const;
      void DeserializeVec2(const std::vector<std::string>& values , glm::vec2& vec) const;
      
      void SerializeVec3(std::ostream& stream , const std::string_view name , const glm::vec3& vec) const;
      void DeserializeVec3(const std::vector<std::string>& values , glm::vec3& vec) const;
      
      void SerializeVec4(std::ostream& stream , const std::string_view name , const glm::vec4& vec) const;
      void DeserializeVec4(const std::vector<std::string>& values , glm::vec4& vec) const;

      void SerializeQuat(std::ostream& stream , const std::string_view name , const glm::quat& vec) const;
      void DeserializeQuat(const std::vector<std::string>& values , glm::quat& vec) const;
  };

#define COMPONENT_SERIALIZERS(name) \
  const std::string GetSerializerName() const { return "ComponentSerializer[" #name "Serializer]"; } \
  virtual void Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const override; \
  virtual void Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const override;


} // namespace other 

#endif // !OTHER_ENGINE_COMPONENT_SERIALIZER_HPP

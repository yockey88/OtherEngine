/**
 * \file ecs/component_serializer.hpp
 **/
#ifndef OTHER_ENGINE_COMPONENT_SERIALIZER_HPP
#define OTHER_ENGINE_COMPONENT_SERIALIZER_HPP

#include "core/config.hpp"
#include "core/ref.hpp"
#include "core/serializer.hpp"

namespace other {

  class Entity;
  class Scene;

  class ComponentSerializer : public Serializer {
    public:
      virtual ~ComponentSerializer() {}

      virtual void Serialize(std::ostream& stream , Entity* owner , const Ref<Scene>& scene) const = 0;
      virtual void Deserialize(Entity* entity , const ConfigTable& scn_table , Ref<Scene>& scene) const = 0;

    protected:
      std::string GetComponentSectionKey(const std::string& name , const std::string& section_key) const;
      
      void SerializeComponentSection(std::ostream& stream , Entity* owner , const std::string_view tag) const;
  };

#define COMPONENT_SERIALIZERS(name) \
  virtual ~name##Serializer() override {} \
  const std::string GetSerializerName() const { return "ComponentSerializer[" #name "Serializer]"; } \
  virtual void Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const override; \
  virtual void Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const override;


} // namespace other 

#endif // !OTHER_ENGINE_COMPONENT_SERIALIZER_HPP

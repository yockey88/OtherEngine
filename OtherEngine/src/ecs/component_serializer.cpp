/**
 * \file ecs/component_serializer.cpp
 **/
#include "ecs/component_serializer.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

#include "core/logger.hpp"

namespace other {
  
  void ComponentSerializer::SerializeVec2(std::ostream& stream , const std::string_view name , const glm::vec2& vec) const {
    stream << name << " = { "
           << vec.x << " , "
           << vec.y
           << " }\n";
  }
      
  void ComponentSerializer::DeserializeVec2(const std::vector<std::string>& values , glm::vec2& vec) const {
    OE_ASSERT(values.size() == 2 , "Attempting to deserialize vec2 with invalid number of values : {} values" , values.size());

    vec.x = std::stof(values[0]);
    vec.y = std::stof(values[1]);
  }

  void ComponentSerializer::SerializeVec3(std::ostream& stream , const std::string_view name , const glm::vec3& vec) const {
    stream << name << " = { "
           << vec.x << " , " 
           << vec.y << " , " 
           << vec.z
           << " }\n";
  }
  
  void ComponentSerializer::DeserializeVec3(const std::vector<std::string>& values , glm::vec3& vec) const {
    OE_ASSERT(values.size() == 3 , "Attempting to deserialize vec3 with invalid number of values : {} values" , values.size());
    
    vec.x = std::stof(values[0]);
    vec.y = std::stof(values[1]);
    vec.z = std::stof(values[2]);
  }
  
  void ComponentSerializer::SerializeVec4(std::ostream& stream , const std::string_view name , const glm::vec4& vec) const {
    stream << name << " = { "
           << vec.x  << " , "
           << vec.y  << " , "
           << vec.z  << " , "
           << vec.w 
           << " }\n";
  }
  
  void ComponentSerializer::DeserializeVec4(const std::vector<std::string>& values , glm::vec4& vec) const {
    OE_ASSERT(values.size() == 4 , "Attempting to deserialize vec4 with invalid number of values : {} values" , values.size());
    
    vec.x = std::stof(values[0]);
    vec.y = std::stof(values[1]);
    vec.z = std::stof(values[2]);
    vec.w = std::stof(values[3]);
  }

  void ComponentSerializer::SerializeQuat(std::ostream& stream , const std::string_view name , const glm::quat& vec) const {
    SerializeVec4(stream , name, { vec.x , vec.y , vec.z , vec.w });
  }
  
  void ComponentSerializer::DeserializeQuat(const std::vector<std::string>& values , glm::quat& vec) const {
    OE_ASSERT(values.size() == 4 , "Attempting to deserialize quaternion with invalid number of values : {} values" , values.size());
    
    vec.x = std::stof(values[0]);
    vec.y = std::stof(values[1]);
    vec.z = std::stof(values[2]);
    vec.w = std::stof(values[3]);
  }


} // namespace other

/**
 * \file ecs/component_serializer.cpp
 **/
#include "ecs/component_serializer.hpp"
#include <cctype>
#include <iterator>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

#include "core/logger.hpp"
#include "ecs/entity.hpp"

namespace other {
      
  std::string ComponentSerializer::GetComponentSectionKey(const std::string& name , const std::string& section_key) const {
    std::string key_name;
    std::transform(name.begin() , name.end() , std::back_inserter(key_name) , ::toupper); 

    key_name += "." + section_key;
    
    OE_DEBUG("Attempting to load {}" , key_name);

    return key_name;
  }
      
  void ComponentSerializer::SerializeComponentSection(std::ostream& stream , Entity* owner , const std::string_view tag) const {
    stream << "[" << owner->Name() << "." << tag << "]\n";
  }
  
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
  
  void ComponentSerializer::SerializeVec2(std::ostream& stream , const std::string_view name , const glm::ivec2& vec) const {
    stream << name << " = { "
           << vec.x << " , "
           << vec.y
           << " }\n";
  }
      
  void ComponentSerializer::DeserializeVec2(const std::vector<std::string>& values , glm::ivec2& vec) const {
    OE_ASSERT(values.size() == 2 , "Attempting to deserialize vec2 with invalid number of values : {} values" , values.size());

    vec.x = std::stoi(values[0]);
    vec.y = std::stoi(values[1]);
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
  
  void ComponentSerializer::SerializeVec3(std::ostream& stream , const std::string_view name , const glm::ivec3& vec) const {
    stream << name << " = { "
           << vec.x << " , " 
           << vec.y << " , " 
           << vec.z
           << " }\n";
  }
  
  void ComponentSerializer::DeserializeVec3(const std::vector<std::string>& values , glm::ivec3& vec) const {
    OE_ASSERT(values.size() == 3 , "Attempting to deserialize vec3 with invalid number of values : {} values" , values.size());
    
    vec.x = std::stoi(values[0]);
    vec.y = std::stoi(values[1]);
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
  
  void ComponentSerializer::SerializeVec4(std::ostream& stream , const std::string_view name , const glm::ivec4& vec) const {
    stream << name << " = { "
           << vec.x  << " , "
           << vec.y  << " , "
           << vec.z  << " , "
           << vec.w 
           << " }\n";
  }
  
  void ComponentSerializer::DeserializeVec4(const std::vector<std::string>& values , glm::ivec4& vec) const {
    OE_ASSERT(values.size() == 4 , "Attempting to deserialize vec4 with invalid number of values : {} values" , values.size());
    
    vec.x = std::stoi(values[0]);
    vec.y = std::stoi(values[1]);
    vec.z = std::stoi(values[2]);
    vec.w = std::stoi(values[3]);
  }

  void ComponentSerializer::SerializeQuat(std::ostream& stream , const std::string_view name , const glm::quat& vec) const {
    SerializeVec4(stream , name, glm::vec4{ vec.x , vec.y , vec.z , vec.w });
  }
  
  void ComponentSerializer::DeserializeQuat(const std::vector<std::string>& values , glm::quat& vec) const {
    OE_ASSERT(values.size() == 4 , "Attempting to deserialize quaternion with invalid number of values : {} values" , values.size());
    
    vec.x = std::stof(values[0]);
    vec.y = std::stof(values[1]);
    vec.z = std::stof(values[2]);
    vec.w = std::stof(values[3]);
  }


} // namespace other

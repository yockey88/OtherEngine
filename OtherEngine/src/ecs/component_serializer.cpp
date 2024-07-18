/**
 * \file ecs/component_serializer.cpp
 **/
#include "ecs/component_serializer.hpp"

#include <iterator>

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

} // namespace other

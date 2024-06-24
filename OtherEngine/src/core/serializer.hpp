/**
 * \file core/serializer.hpp
 **/
#ifndef OTHER_ENGINE_SERIALIZER_HPP
#define OTHER_ENGINE_SERIALIZER_HPP

#include <string_view>
#include <ostream>
#include <vector>

#include <glm/glm.hpp>

namespace other {

  class Serializer {
    protected:
      template <typename T>
      void SerializeValue(std::ostream& stream , const std::string_view name , T val) const {
        stream << name << " = " << val << "\n";
      }

      template <>
      void SerializeValue(std::ostream& stream , const std::string_view name , bool val) const {
        stream << name << " = "
               << (val ? "true" : "false") 
               << "\n";
      }

      void SerializeVec2(std::ostream& stream , const std::string_view name , const glm::vec2& vec) const;
      void DeserializeVec2(const std::vector<std::string>& values , glm::vec2& vec) const;
      
      void SerializeVec2(std::ostream& stream , const std::string_view name , const glm::ivec2& vec) const;
      void DeserializeVec2(const std::vector<std::string>& values , glm::ivec2& vec) const;
      
      void SerializeVec3(std::ostream& stream , const std::string_view name , const glm::vec3& vec) const;
      void DeserializeVec3(const std::vector<std::string>& values , glm::vec3& vec) const;
      
      void SerializeVec3(std::ostream& stream , const std::string_view name , const glm::ivec3& vec) const;
      void DeserializeVec3(const std::vector<std::string>& values , glm::ivec3& vec) const;
      
      void SerializeVec4(std::ostream& stream , const std::string_view name , const glm::vec4& vec) const;
      void DeserializeVec4(const std::vector<std::string>& values , glm::vec4& vec) const;
      
      void SerializeVec4(std::ostream& stream , const std::string_view name , const glm::ivec4& vec) const;
      void DeserializeVec4(const std::vector<std::string>& values , glm::ivec4& vec) const;

      void SerializeQuat(std::ostream& stream , const std::string_view name , const glm::quat& vec) const;
      void DeserializeQuat(const std::vector<std::string>& values , glm::quat& vec) const;
  };

} // namespace other

#endif // !OTHER_ENGINE_SERIALIZER_HPP

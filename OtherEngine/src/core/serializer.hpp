/**
 * \file core/serializer.hpp
 **/
#ifndef OTHER_ENGINE_SERIALIZER_HPP
#define OTHER_ENGINE_SERIALIZER_HPP

#include <string_view>
#include <ostream>
#include <vector>

#include <glm/glm.hpp>

#include "core/uuid.hpp"
#include "asset/asset_types.hpp"

namespace other {

  class Serializer {
    protected:
      template <typename T>
      void SerializeValue(std::ostream& stream , const std::string_view name , T val) const {
        stream << name << " = " << val << "\n";
      }
      
      template <>
      void SerializeValue(std::ostream& stream , const std::string_view name , UUID val) const {
        stream << name << " = " << val.Get() << "\n";
      }
      
      template <>
      void SerializeValue(std::ostream& stream , const std::string_view name , AssetHandle val) const {
        stream << name << " = " << val.Get() << "\n";
      }

      template <typename T>
      void SerializeList(std::ostream& stream , const std::string_view name , const std::vector<T>& values) const {
        stream << name << " = {\n  ";
        for (auto& v : values) {
          stream << v;
          if (v != values.end()) {
            stream << " , ";
          } else {
            stream << "\n";
          }
        }
        stream << "}\n";
      }
      
      template <>
      void SerializeList(std::ostream& stream , const std::string_view name , const std::vector<UUID>& values) const {
        stream << name << " = {\n  ";
        for (uint32_t i = 0; i < values.size(); ++i) {
          stream << values[i].Get();
          if (i < values.size() - 1) {
            stream << " , ";
          } else {
            stream << "\n";
          }
        }
        stream << "}\n";
      }
      
      template <>
      void SerializeList(std::ostream& stream , const std::string_view name , const std::vector<AssetHandle>& values) const {
        stream << name << " = {\n  ";
        for (uint32_t i = 0; i < values.size(); ++i) {
          stream << values[i].Get();
          if (i < values.size() - 1) {
            stream << " , ";
          } else {
            stream << "\n";
          }
        }
        stream << "}\n";
      }

      // template <size_t N , typename... Args>
      // void SerializeItems(std::ostream& stream , const std::array<std::string_view , N> names , Args&&... items) const {
      // }

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

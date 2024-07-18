/**
 * \file scene/scene_serializer.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_SERIALIZER_HPP
#define OTHER_ENGINE_SCENE_SERIALIZER_HPP

#include "core/ref.hpp"
#include "core/config.hpp"
#include "core/serializer.hpp"

#include "scene/scene.hpp"

namespace other {

  struct DeserializedScene {
    Ref<Scene> scene = nullptr;
    ConfigTable scene_table;
    std::string name = "[ Empty Scene ]";
  };

  class SceneSerializer : public Serializer {
    public: 
      SceneSerializer() {}
      ~SceneSerializer() {}

      void Serialize(const std::string_view scene_name , std::ostream& stream , const Ref<Scene>& scene) const;
      DeserializedScene Deserialize(const std::string_view scn_path) const;

    private:
  };

} // namespace other

#endif // !OTHER_ENGINE_SCENE_SERIALIZER_HPP

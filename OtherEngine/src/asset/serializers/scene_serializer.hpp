/**
 * \file asset/serializers/scene_serializer.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_SERIALIZER_HPP
#define OTHER_ENGINE_SCENE_SERIALIZER_HPP

#include "core/byte_buffer.hpp"
#include "core/config.hpp"
#include "core/ref.hpp"

#include "asset/asset_serializer.hpp"

#include "scene/scene.hpp"

#include "serialization/scene_file_format_defines.hpp"

namespace other {

  struct DeserializedScene {
    Ref<Scene> scene = nullptr;
    Path path;
    ConfigTable scene_table;
    std::string name = "[ Empty Scene ]";
  };

  class SceneSerializer : public AssetSerializer {
   public:
    SceneSerializer() {}
    virtual ~SceneSerializer() override {}

    virtual void Serialize(const AssetMetadata& metadata) override;
    virtual bool Load(AssetMetadata& metadata) override;

    static void Write(ByteBuffer& buffer, const Ref<Scene>& scene);
    static void Read(ByteBuffer& buffer, Ref<Scene>& scene);

   private:
    void Serialize(const std::string_view scene_name, std::ostream& stream, const Ref<Scene>& scene) const;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_SCENE_SERIALIZER_HPP
